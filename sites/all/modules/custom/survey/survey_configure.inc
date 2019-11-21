<?php
/**
 * Created by PhpStorm.
 * User: mende
 * Date: 12/6/2017
 * Time: 10:02 PM
 */

/**
 * Builds the form to manage survey settings.
 */
function survey_configure_form($form, &$form_state) {

  $survey = _survey_get_survey(1);

  $form['title'] = array(
    '#type' => 'textarea',
    '#rows' => 1,
    '#resizable' => FALSE,
    '#title' => t("Umfragetitel"),
    '#maxlength' => 254,
    '#default_value' => $survey['title'],
  );
    /*
  $form['links']['user_feedback'] = array(
    '#type' => 'link',
    '#title' => t('Benutzer-Anmerkungen anzeigen'),
    '#href' => 'survey/feedback',
  );
    */



  $form['welcome'] = array(
    '#type' => 'text_format',
    '#title' => t('Willkommensnachricht'),
    '#default_value' => $survey['welcome_msg'],
    '#format' => 'full_html',
    '#rows' => 16,
  );

  $form['end'] = array(
    '#type' => 'text_format',
    '#title' => t('Endnachricht'),
    '#default_value' => $survey['end_msg'],
    '#format' => 'full_html',
    '#rows' => 6,
  );

  $form['threshold'] = array(
      '#type' => 'select',
      '#title' => t('Threshold'),
      '#options' => array(
        0 => 1,
        1 => 2,
        2 => 3,
        3 => 4,
        4 => 5,
      ),
      '#description' => t('This sets the threshold for displaying the boxplot.')
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Speichern'),
  );

  // automatically resize textareas to content
  $form = delphi_question_add_autosize($form);

  return $form;
}

/**
 * Implements hook_form_submit().
 *
 * Saves survey settings to database.
 *
 * @throws \InvalidMergeQueryException
 */
function survey_configure_form_submit($form, &$form_state) {
  $numUpdated = db_merge('survey')
    ->fields(array(
      'title' => $form_state['values']['title'],
      'welcome_msg' => $form_state['values']['welcome']['value'],
      'end_msg' => $form_state['values']['end']['value']))
    ->condition('survey_id', 1, '=')
    ->execute();

  $threshold = $form_state['values']['threshold']['value'];
  variable_set('delphi_threshold', $threshold);

  if ($numUpdated != 0) {
    drupal_set_message(t('Umfrage wurde aktualisiert.'));
  } else {
    drupal_set_message(t('Es wurden keine Änderungen vorgenommen.'));
  }
}

/**
 * Displays all submitted user feedback.
 */
function survey_show_feedback_form($form, &$form_state) {

  try {
    $result = db_query("SELECT feedback, feedback_date FROM {survey_users} WHERE (feedback != null OR feedback != '')
                              ORDER BY feedback_date");
    $feedback = $result->fetchAllAssoc('feedback_date');

  } catch (PDOException $e) {
    $feedback = array();
  }

  $html = '';
  if (!empty($feedback)) {
    foreach ($feedback as $date => $remark) {
      $html .= '<p>[' . $date . '] ' . check_plain($remark->feedback) . '</p>';
    }
  }

  if (empty($html)) {
    $html = t("Es wurden noch keine Anmerkungen übermittelt.");
  }

  $form['feedback']= array(
    '#markup' => $html,
    '#attributes' => array(
      'class' => array('user-feedback'),
    )
  );

  return $form;
}

/**
 * Displays all submitted user experts.
 */
function survey_show_feedback_experts_form($form, &$form_state) {

  try {
    $result = db_query("SELECT feedback_experts
                        FROM {survey_users}
                        WHERE (feedback_experts != null OR feedback_experts != '')
                        ");
    $experts = $result->fetchAll();

  } catch (PDOException $e) {
    $experts = array();
  }

  $html = '';
  if (!empty($experts)) {
    foreach ($experts as $expert) {
      $html .= '<p>' . check_markup($expert->feedback_experts) . '</p>';
    }
  }

  if (empty($html)) {
    $html = t("Es wurden noch keine Experten vorgeschlagen.");
  }

  $form['feedback']= array(
    '#markup' => $html,
    '#attributes' => array(
      'class' => array('user-feedback'),
    )
  );

  return $form;
}


