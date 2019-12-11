<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 16.11.18
 * Time: 17:14
 */

/*
 * Diese Funktion führt den Nutzer auf die Abschlusseite der Umfrage.
 */
function survey_finish_survey_form() {
    $userId = arg(1);

    $survey = _survey_get_survey(1);
    $user = _survey_get_user($userId);

    if (empty($user)) {
        form_set_error('Survey', 'Invalid User');
        return null;
    }

    $form = array();
    $form['end_msg'] = array(
        '#markup' => $survey['end_msg'],
        '#attributes' => array(
            'id' => 'survey-end',
        ),
    );

    $form['feedback'] = array(
        '#title' => t('Anmerkungen'),
        //'#description' => t('Falls Sie Anmerkungen haben.'),
        '#resizable' => FALSE,
        '#type' => 'textarea',
        '#default_value' => $user['feedback'],
        '#attributes' => array(
            'id' => 'survey-feedback',
        ),
    );

    /*
    $form['experts'] = array(
      '#title' => t('Kennen Sie weitere Experten? (Name, E-Mail)'),
      '#resizable' => FALSE,
      '#type' => 'textarea',
      '#rows' => 2,
      '#default_value' => $user['feedback_experts'],
      '#attributes' => array(
        'id' => 'survey-feedback-experts',
      ),
    );
    */
    $form = delphi_question_add_autosize($form);

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Anmerkungen speichern',
    );

    return $form;
}

/**
 * Implements hook_form_submit().
 */
function survey_finish_survey_form_submit($form, &$form_state) {
    $userId = arg(1);
    $success = FALSE;

    $userFeedback = $form_state['values']['feedback'];
    $userExperts = $form_state['values']['experts'];
    $success = _survey_save_feedback($userId, $userFeedback, $userExperts);


    if ($success) {
        drupal_set_message('Ihre Anmerkungen wurden gespeichert. Herzlichen Dank.');
    }
}
