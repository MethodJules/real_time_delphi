<?php
/**
 * Created by PhpStorm.
 * User: jonaskortum
 * Date: 24.07.17
 * Time: 11:39
 */

/*
 * Diese Funktion baut eine Seite, die in einer Tabelle alle Fragen enthält
 */
function delphi_question_question_overview_form($form, &$form_state) {
  global $base_url;


  $form['links']['new_questiongroup'] = array(
    '#type' => 'link',
    '#title' => t('Themenbereich hinzufügen'),
    '#href' => 'add_group/group',
  );

  $form['links']['new_question'] = array(
    '#type' => 'link',
    '#title' => t('These hinzufügen'),
    '#href' => 'add_questions/1',
  );



  $form['table'] = array(
    '#type' => 'markup',
    '#title' => t('Thesenübersicht'),
    '#prefix' => '<table class="select-table">',
    '#suffix' => '</table>'
  );

  //Kopf der Tabelle wird erstellt
  $form['table']['header'] = array(
    '#type' => 'markup',
    '#markup' => "<th>Nr.</th>
            <th>These</th>
            <th>Aktion</th>",
  );

  //Fragen werden geladen
  $question_result = db_query("SELECT * FROM {question} ORDER BY weight, question_id");
  $noQuestions = $question_result->rowCount();

  $id = 1;
  foreach ($question_result as $question) {

    //Antworten der Fragen werden geladen
    $answer_result = db_query("SELECT * FROM {question_possible_answers} WHERE question_id = :question_id", array(
      ':question_id' => $question->question_id
    ));

    //Die Antworten werden gezählt
    $quan = $answer_result->rowCount();

    // Tabellenzeile aufbauen
    $rowClass = 'question-row';

    if ($question->type === 'group') {
      $rowClass = 'question-group-row';
    }

    $form['table']['rows'][$id] = array(
      '#type' => 'markup',
      '#prefix' => '<tr class="' . $rowClass . '">',
      '#suffix' => '</tr>',
    );

    // Auswahl der Fragengewichtung
    $options = drupal_map_assoc(range(1, $noQuestions));
    $form['table']['rows'][$id]['weight' . $id] = array(
      '#type' => 'select',
      '#title' => t(''),
      '#options' => $options,
      '#default_value' => $id,
      '#id' => $question->question_id,
      '#attributes' => array(
        'class' => array('weight-table-question'),
        'onChange' => 'this.form.submit();',
      ),
      '#prefix' => '<td>',
      '#suffix' => '</td>',
    );

    $form['table']['rows'][$id]['title'] = array(
      '#type' => 'markup',
      '#titel' => '',
      '#markup' => '<td>' . check_plain($question->title) . '</td>',
    );

    //Links zum Ändern und Löschen der Frage werden erstellt
    $linkDelete = $base_url . "/?q=delete_question/" . $question->question_id;
    $linkEdit = $base_url . "/?q=change_question/" . $question->question_id . "/" . $quan;

    $form['table']['rows'][$id]['links'] = array(
      '#type' => 'markup',
      '#markup' => '<td>'
        . l('bearbeiten', $linkEdit)
        . '<br />'
        . l('löschen', $linkDelete)
        . '</td>'
    );

    $id++;
  }


  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Geänderte Reihenfolge speichern',
    '#attributes' => array(
      'style' => array('display: none;'),
    ),
  );

  drupal_set_title('Thesenübersicht');

  return $form;
}


/*
 * Diese Funktion löscht eine Frage aus der Datenbank
 */
function delphi_question_delete_question()
{
    //ID der Frage wird ermittelt
    $question_id = arg(1);

    //Frage wird gelöscht
    db_query("DELETE FROM {question} WHERE question_id = :question_id", array(
        ':question_id' => $question_id
    ));

    //Antwortmöglichkeiten der Frage werden gelöscht
    db_query("DELETE FROM {question_possible_answers} WHERE question_id = :question_id", array(
        ':question_id' => $question_id
    ));

    //Antworten werden aus der Beantwortungstabelle entfernt
    db_query("DELETE FROM {question_user_answers} WHERE question_id = :question_id", array(
        ':question_id' => $question_id
    ));

    drupal_set_message("These wurde gelöscht");
    drupal_goto("question_overview");
}


/**
 * Implements drupal_form_submit().
 *
 * Saves the changed question ordering.
 */
function delphi_question_question_overview_form_submit($form, &$form_state) {
  foreach ($form['table']['rows'] as $key => $row) {
    if(isset($row['weight' . $key])) {
      $rowData = $row['weight' . $key];
      $weightOld = $rowData['#default_value'];
      $weightNew = $rowData['#options'][$rowData['#value']];

      // Update only if the weight changed.
      // Break on first change since only one weight change is possible at a time (form gets updated).
      if ($weightNew !== $weightOld) {
        $questionId = $rowData['#id'];
        delphi_question_update_question_order($questionId, $weightOld, $weightNew);
        break;
      }
    }
  }

  drupal_goto("question_overview");
}


/**
 * Updates the weight of a question and dependent questions to change the question order.
 *
 * @param $questionId
 *   The ID of the question to be updated.
 * @param $weightOld
 *   The question weight before the update.
 * @param $weightNew
 *   The new question weight.
 */
function delphi_question_update_question_order($questionId, $weightOld, $weightNew) {

  // Updates all question weights that are influenced by the new ordering (question in between the old und new weight).
  if ($weightNew > $weightOld) {
    while ($weightNew !== $weightOld) {
      $qId = delphi_question_get_question_by_weight($weightOld + 1);
      delphi_question_update_question_weight($qId, $weightOld);
      $weightOld++;
    }

    delphi_question_update_question_weight($questionId, $weightNew);

  } else if ($weightNew < $weightOld) {
    while ($weightNew !== $weightOld) {
      $qId = delphi_question_get_question_by_weight($weightOld - 1);
      delphi_question_update_question_weight($qId, $weightOld);
      $weightOld--;
    }

    delphi_question_update_question_weight($questionId, $weightNew);
  }
}


/**
 * Returns the questions ID that matches the given weight.
 *
 * @param $weight
 *   Question weight to look for.
 *
 * @return int
 *   Question ID.
 */
function delphi_question_get_question_by_weight($weight) {
  $id = db_query("SELECT question_id FROM {question} WHERE weight = :w",
    array(':w' => $weight))->fetchField();

  return $id;
}


/**
 * Updates the weight of a question.
 *
 * @param $questionId
 *   ID of the question to be updated
 * @param $newWeight
 *   New question weight.
 */
function delphi_question_update_question_weight($questionId, $newWeight) {
  db_update('question')
    ->fields(array(
      'weight' => $newWeight,
    ))
    ->condition('question_id', $questionId, '=')
    ->execute();
}