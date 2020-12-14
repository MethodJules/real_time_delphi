<?php
/**
 * Created by PhpStorm.
 * User: jonaskortum
 * Date: 16.02.17
 * Time: 10:48
 */

require_once("survey_create_boxplot_data.inc");
require_once("survey_create_boxplot.inc");

/*
 * Diese Funktion erstellt eine Seite, auf der die Frage mit allen Antwortmöglichkeiten und dem Boxplot angezeigt wird
 */
function survey_question_evaluation_form()
{

    //Die ID der Frage wird aus der URL gelesen
    $question_id = arg(1);

    //Die ID des aktuellen Benutzers wird aus der URL gelesen
    $user_id = arg(2);

    //Es werden alle Fragen aus der Datenbank gelesen
    $questions = survey_get_all_questions();
    $question_index = 0;
    $questionGroupIndex = 0;
    $questionType = 'question';

    foreach ($questions as $q) {
        if ($q->question_id == $question_id) {
          $questionType = $q->type;
          break;
        }

        if ($q->type === 'group') {
          $questionGroupIndex++;
        }
        $question_index++;
    }

    // Fehlermeldung bei unbekannter question_id
    if (count($questions) === $question_index) {
      form_set_error('Question', 'Invalid ID');
      return null;
    }

    // no evauluation page for question groups
    if ($questionType === 'group') {
      drupal_goto("survey_question/" . $question_id . "/" . $user_id);
    }


    //DIe Überschrift wird erstellt
    $headline = $questions[$question_index]->title;
    $titleLength = drupal_strlen($headline);
    $headline = "
        <h1>These " . ($question_index -$questionGroupIndex + 1) . "</h1>
        <h2>" . $headline . "</h2>";

    $stringSelfAssessment = t('Hier können Sie Ihre Einschätzungen mit den gemittelten Werten der anderen Teilnehmer/innen vergleichen 
    und deren Kommentare einsehen. Sie können daraufhin bei Ihrer Einschätzung bleiben oder diese verändern. 
    Wenn Sie möchten, können Sie Ihre Einschätzung in einem eigenen Kommentar begründen oder bedenkenswerte Hinweise geben.');

    //$stringSelfAssessment = t('Hier können Sie Ihre Einschätzung bewerten');    
    $form['headline']['title'] = array(
        '#type' => 'markup',
        '#title' => t('Headline'),
        '#markup' => $headline,
        '#prefix' => '<div class="question-header">',
    );
    /*
    $form['boxplot']['info'] = array(
      '#theme' => 'table',
      '#header' => array('Kennzahl', 'Beschreibung', 'Lage im Boxplot'),
      '#rows' => array(
          array('Minimum', 'Kleinster Wert des Datensatzes', 'Ende eines Whiskers |--'),
          array('Unteres Quartil', 'Die kleinsten 25 % der Datenwerte sind kleiner als dieser oder gleich diesem Kennwert', 'Beginn der Box'),
      ),
      '#attributes' => array('class' => array('boxplot-description-table ')), 
    );
    */


    $htmlSelfAssessment = '<div class="self-assessment">' . check_plain($stringSelfAssessment) . '</div>';

    // Da Titel immer fix im Vordergrund steht muss die Content-Region um die Höhe des Titels verschoben werden
    $cssClassContent = 'evaluation-content-wrapper';

    if ($titleLength > 300) {
      $form['headline']['title']['#suffix'] = $htmlSelfAssessment. '</div><div class="four-rows ' . $cssClassContent . '">';
    } else if ($titleLength > 200) {
      $form['headline']['title']['#suffix'] = $htmlSelfAssessment .'</div><div class="three-rows ' . $cssClassContent . '">';
    } else if ($titleLength > 100) {
      $form['headline']['title']['#suffix'] = $htmlSelfAssessment .'</div><div class="two-rows ' . $cssClassContent . '">';
    } else {
      $form['headline']['title']['#suffix'] = $htmlSelfAssessment .'</div><div class="one-row ' . $cssClassContent . '">';
    }


    //In diese Variable werden alle Antwortmöglichkeiten der Frage eingespeichert
    $answers = array();

    //TODO Check for refactoring
    //SQL-> Get lables for radio buttons

    $button_title_result = db_query("SELECT * FROM {question_buttons_title} WHERE question_id = :question_id ORDER BY answer_id, button_id", array(
        ':question_id' => $questions[$question_index]->question_id
    ));

    $button_title_array = survey_get_button_titles($button_title_result);

    //Die gegebenen Antworten des Nutzers werden geladen
    $answers_result = db_query("SELECT * FROM {question_user_answers} WHERE user_pw = :user_pw AND question_id = :question_id", array(
        ':user_pw' => $user_id,
        ':question_id' => $question_id
    ));

    // Keine Antworten des Benutzers gefunden
    if ($answers_result->rowCount() === 0) {
      form_set_error('Answers', 'Invalid User');
      return null;
    }

    //Die Antworten werden ausgelesen und in ein Array gespeichert
    foreach ($answers_result as $answer) {

        $check = survey_question_evaluation_search_for_answer_ID($answer->answer_id, $answers);

        if ($check > -1) {
            $answers[$check] = $answer;
        } else {
            array_push($answers, $answer);
        }
    }

    //Es wird über alle Antworten iteriert (graues Feld in der UI)
  $countTextfield = 0;
  for ($i = 0; $i < sizeof($answers); $i++) {

        //Es werden alle Fragen aus der Datenbank ausgelesen
        $questions = survey_get_all_questions();

        //Die aktuelle Fragen-ID wird ermittelt
        $question_id = $questions[$question_index]->question_id;

        //Die Antwortmöglichkeiten der Frage werden geladen
        $possible_answers_result = db_query("SELECT * FROM {question_possible_answers} WHERE question_id = :question_id ORDER BY weight", array(
            ':question_id' => $question_id
        ));

        //In dieses Array werden die Antwortmöglichkeiten der entsprechenden Frage gespeichert
        $possible_answers_array = array();

        //Es wird über alle Antwortmöglichkeiten der Frage iteriert
        foreach ($possible_answers_result as $possible_answer) {
            array_push($possible_answers_array, $possible_answer);
        }

        $form['fieldset' . $i] = array(
            '#type' => 'fieldset',
        );

        //Sollte die aktuelle Antwortmöglichkeit Radio-Buttons beinhalten:
      // TODO duplicate code
      $questionType = $possible_answers_array[$i]->question_type;
      if ($possible_answers_array[$i]->isRadioButton) {

            //Der Boxplot zur Antwort wird geladen
            // Index der Antwortoption "keine Ahnung"
            $indexNoAnswer = count($button_title_array[$i - $countTextfield]) - 1;
            $boxplot_graphic = get_graphic($i, 1, $indexNoAnswer); // TODO quick fix

            $answerDescription = $possible_answers_array[$i]->description;
            $form['fieldset' . $i]['#attributes'] = array(
              'class' => array('evaluation', $questionType));
            $form['fieldset' . $i]['#title'] = $answerDescription;

            // Fieldset legend vergrößern bei langen Titeln
            if (drupal_strlen($answerDescription) > 97) {
              $form['fieldset' . $i]['#attributes'] = array(
                'class' => array('title-long'));
            }

            $form['fieldset' . $i]['my_markup' . $i] = array(
                '#markup' => $boxplot_graphic,
            );

            //dsm($button_title_array[$i - $countTextfield]);

            $form['fieldset' . $i]['dim' . $i] = array(
                '#type' => 'radios',
                '#title' => '',
                '#options' => $button_title_array[$i - $countTextfield], // TODO quick fix
                '#default_value' => $answers[$i]->answer,
                '#attributes' => array(
                    'class' => array('radios-' . count($button_title_array[$i - $countTextfield]))
                ),
            );

            $form['fieldset' . $i]['comment_dim' . $i] = array(
                '#title' => t('Kommentar'),
                '#resizable' => FALSE,
                '#type' => 'textarea',
                '#default_value' => $answers[$i]->comment,
            );

            $form = _survey_addUserComments($form, $i, $question_id, $answers[$i]->answer_id, $user_id, $button_title_array[$i - $countTextfield]);

        //Sollte die aktuelle Antwortmöglichkeit eine freie Antwort beinhalten:
        } else if ($questionType === 'year'){
            $countTextfield++;
            //Der Boxplot zur Antwort wird geladen
            $boxplot_graphic = get_graphic($i, 0, array('Min', 'Max'));

            $form['fieldset' . $i]['#attributes'] = array(
              'class' => array('evaluation', $questionType));

            $form['fieldset' . $i]['#title'] = $possible_answers_array[$i]->description;

            $form['fieldset' . $i]['my_markup' . $i] = array(
                '#markup' => $boxplot_graphic,
            );

            // 'answer_NA' ist interne Codierung für 'keine Angabe' und soll nicht angezeigt werden.
            $isChecked = FALSE;
            if ($answers[$i]->answer === 'answer_NA') {
              $answers[$i]->answer = '';
              $isChecked = TRUE;
            }

            $form['fieldset' . $i]['answer' . $i] = array(
                '#type' => 'textfield',
                '#title' => $possible_answers_array[$i]->description,
                '#title_display' => 'invisible',
                '#size' => 4,
                '#maxlength' => 4,
                '#required' => FALSE,
                '#default_value' => $answers[$i]->answer,
                '#states'        => array(
                  'disabled' => array(
                    ':input[name="answer_check' . $i . '"]' => array('checked' => TRUE),
                  ),
                ),
                '#element_validate' => array('survey_textfield_validate'),
                '#attributes' => array('class' => array($questionType)),
            );

            $form['fieldset' . $i]['answer_check' . $i] = array(
              '#type' => 'checkbox',
              '#title' => 'weiß nicht / keine Angabe',
              '#attributes' => array(
                'class' => array('text-checkbox')
              ),
              '#default_value' => $isChecked,
            );

            $form['fieldset' . $i]['comment_dim' . $i] = array(
                '#title' => t('Kommentar'),
                '#resizable' => FALSE,
                '#type' => 'textarea',
                '#default_value' => $answers[$i]->comment,
            );

            $form = _survey_addUserComments($form, $i, $question_id, $answers[$i]->answer_id, $user_id, array());

      } else if ($questionType === 'text') {
        $countTextfield++;

        $form['fieldset' . $i]['#title'] = $possible_answers_array[$i]->description;
        $form['fieldset' . $i]['#attributes'] = array('class' => array('evaluation', $questionType));

        // evaluation data
        $evalAnswers = db_query("SELECT answer FROM {question_user_answers} 
              WHERE question_id = :question_id 
              AND answer_id = :answer_id
              AND user_pw != :user_pw
              AND is_last_answer = 1", array(
          ':question_id' => $question_id,
          ':answer_id' => $answers[$i]->answer_id,
          ':user_pw' => $user_id,
          ))->fetchCol();

        $noEvalAnswers = count($evalAnswers);
        if ($noEvalAnswers >= 1) { //TODO checken
          $labelNa = t('weiß nicht / keine Angabe');
          $evalAnswers = str_replace('answer_NA', $labelNa, $evalAnswers);

          $form['fieldset' . $i]['evaluation' . $i] = array(
            '#theme' => 'item_list',
            '#list_type' => 'ul',
            '#items' => $evalAnswers,
            '#attributes' => array('class' => array($questionType)),
          );

        } else {
          $evalHtml = '<div class = "boxplot-nodata">' .
            t('Zu diesem Thema wurden noch nicht genügend Einschätzungen abgegeben.') .
            '</div>';

          $form['fieldset' . $i]['insufficient_data' . $i] = array(
            '#markup' => $evalHtml,
          );
        }

        // user's answer TODO duplicate code => universal function for form elements skeleton
        // 'answer_NA' ist interne Codierung für 'keine Angabe' und soll nicht angezeigt werden.
        $isChecked = FALSE;
        if ($answers[$i]->answer === 'answer_NA') {
          $answers[$i]->answer = '';
          $isChecked = TRUE;
        }

        $form['fieldset' . $i]['answer' . $i] = array(
          '#type' => 'textarea',
          '#title' => $possible_answer->description,
          '#title_display' => 'invisible',
          '#resizable' => FALSE,
          '#rows' => 2,
          '#default_value' => $answers[$i]->answer,
          '#required' => FALSE,
          '#states'        => array(
            'disabled' => array(
              ':input[name="answer_check' . $i . '"]' => array('checked' => TRUE),
            ),
          ),
          '#element_validate' => array('_survey_textarea_validate'),
          '#attributes' => array('class' => array($questionType)),
        );

        $form['fieldset' . $i]['answer_check' . $i] = array(
          '#type'          => 'checkbox',
          '#title'         => 'weiß nicht / keine Angabe',
          '#attributes'    => array('class' => array($questionType)),
          '#default_value' => $isChecked,
        );

        // comments of others users
        $form['fieldset' . $i]['comment_dim' . $i] = array(
          '#title'         => t('Kommentar'),
          '#resizable'     => FALSE,
          '#type'          => 'textarea',
          '#default_value' => $answers[$i]->comment,
        );

        $form = _survey_addUserComments($form, $i, $question_id, $answers[$i]->answer_id, $user_id, array());
      }

    }

    $form['content-wrapper-close'] = array(
      '#type' => 'markup',
      '#markup' => '</div>',
    );

    $button_text = "Weiter zur nächsten These";

    //Die ID der nächsten Frage wird ermittelt
    $id = survey_question_evaluation_get_next_question(arg(1));

    //Sollte es die letzte Frage sein, wird die Umfrage beendet
    if ($id == -1) {
      $button_text = "Umfrage abschließen";
    }

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => $button_text,
        '#submit' => array('survey_get_next_question')
      );

    //Sollte es sich nicht um die erste Frage der Umfrage handeln, wird ein Zurück-Button erstellt
    if ($question_id !== 0 && $question_index !== 0) {
        $form['back'] = array(
            '#type' => 'submit',
            '#value' => 'Zurück',
            '#limit_validation_errors' => array(),
            '#submit' => array('survey_question_evaluation_get_back_to_last_question')
        );
    }

    // The survey progress bar
    $form['progress_bar'] = array(
      '#markup' => _survey_get_status_bar($user_id),
    );

    // Drupal error messages mess with the fixed quesstion headers. Unset fixed if error messages are visible.
    $form['#attached']['js'][] = array(
      'type' => 'inline',
      'data' => "jQuery(document).ready(function () {if (jQuery('.messages.error').length) {
            jQuery('.question-header').css('position', 'unset');
            jQuery('.question-content-wrapper').css('padding-top', 0);
          }
          });",
      'scope' => 'footer',
    );

    return $form;
}

function survey_question_evaluation_get_back_to_last_question(&$form)
{
    $question_id = arg(1);
    $user_pw = arg(2);


    //Es werden alle Fragen aus der Datenbank gelesen
    $questions = survey_get_all_questions();
    $question_index = 0;

    foreach ($questions as $q) {
        if ($q->question_id == $question_id) {
            break;
        }
        $question_index++;
    }

    // "Zurück" führt immer nur zur Überprüfungsseite der vorherigen Frage zurück (nicht zur Umfrageseite)
    $gotoPath = 'survey_question_evaluation/';

    if ($question_index != 0) {
        drupal_goto($gotoPath . $questions[$question_index - 1]->question_id . "/" . $user_pw);
    } else {
        drupal_goto($gotoPath . $questions[$question_index]->question_id . "/" . $user_pw);
    }
}

/*
 * Diese Funktion überprüft, ob die Antwort existiert
 */
function survey_question_evaluation_search_for_answer_ID($id, $array)
{

    $check = -1;

    foreach ($array as $key => $val) {
        if (!strcmp($val->answer_id, $id)) {
            $check = $key;
        }
    }
    return $check;
}

/*
 * Diese Funktion speichert die eingegebenen Daten des Nutzers in die Datenbank und führt zur nächsten Frage, falls
 * noch eine vorhanden ist. Andernfalls wird die Umfrage beendet
 */
function survey_get_next_question($form, &$form_state)
{

    //Die ID der aktuellen Frage wird aus der URL geladen
    $question = arg(1);

    //Die ID des aktuellen Nutzers wird aus der URL geladen
    $user_id = arg(2);

    //Es werden alle Fragen aus der Datenbank geladen
    $questions = survey_get_all_questions();

    $question_index = 0;

    foreach ($questions as $q) {
        if ($q->question_id == $question) {
            break;
        }
        $question_index++;
    }

    //Es werden alle Antwortmöglichkeiten aus der Datenbank geladen
    $answers = survey_get_answers_from_question($questions[$question_index]->question_id);


    //Es wird über alle Antwortmöglichkeiten iteriert
    for ($i = 0; $i < sizeof($answers); $i++) {

        //Die Eingaben des Nutzers werden erfasst und in eine Variable gespeichert
        if ($answers[$i]->isRadioButton) {
            $dim1 = $form_state['values']['dim' . $i];
        } else {
            $dim1 = $form_state['values']['answer' . $i];
        }

        $comment = $form_state['values']['comment_dim' . $i];

        //Die aktuelle Antwort des Nutzers zur Antwort wird aus der Datenbank geladen
        $result = db_query("SELECT * FROM {question_user_answers} WHERE user_pw = :user_pw AND question_id = :question_id AND answer_id = :answer_id ORDER BY question_user_answers_id DESC LIMIT 1", array(
            ':user_pw' => $user_id,
            ':question_id' => $question,
            ':answer_id' => $i
        ));

        foreach ($result as $item) {

            // Wurde die Antwort nicht geändert, nur die Kommentare zu den vorhandenen Antworten speichern
            if (!strcmp($dim1, $item->answer)) {
                db_query("UPDATE {question_user_answers} SET comment = :comment WHERE user_pw = :user_pw AND question_id = :question_id AND answer_id = :answer_id AND is_last_answer = 1", array(
                    ':comment' => $comment,
                    ':user_pw' => $user_id,
                    ':question_id' => $question,
                    ':answer_id' => $i

                ));

            // geänderte Antworten neu speichern und alte Antworten beibehalten
            } else {
                db_query("UPDATE {question_user_answers} SET is_last_answer = 0 WHERE answer_id = :answer_id AND question_id = :question_id AND user_pw = :user_pw AND is_last_answer = 1", array(
                  ':answer_id'   => $i,
                  ':question_id' => $question,
                  ':user_pw'     => $user_id
                ));

                db_insert('question_user_answers')
                    ->fields(array(
                        'question_id' => $question,
                        'answer_id' => $i,
                        'answer' => $dim1,
                        'user_pw' => $user_id,
                        'comment' => $comment,
                        'is_last_answer' => 1,
                    ))
                    ->execute();
            }
        }
    }

    //Es wird ermittelt, ob es eine weitere Frage gibt. Wenn ja, wird zur nächsten Frage weitergeleitet, andernfalls
    //wird die Umfrage beendet
    $id = survey_question_evaluation_get_next_question($question);

    if ($id == -1) {
        drupal_goto("finish_survey/" . $user_id);
    } else {
        drupal_goto("survey_question/" . $id . "/" . $user_id);
    }

}

/*
 * Diese Funktion ermittelt anhand der aktuellen Fragen-ID die nächste
 */
function survey_question_evaluation_get_next_question($question_id)
{
    $questions = array();
    $result = db_query("SELECT * FROM {question} ORDER BY weight, question_id");

    foreach ($result as $item) {
        array_push($questions, $item->question_id);
    }

    $in = array_search($question_id, $questions);

    if (sizeof($questions) > $in + 1) {

        return $questions[$in + 1];
    } else {
        return -1;
    }
}

/*
 * Diese Funktion gibt die Titel der Button für Antwortmöglichkeiten zurück
 */
function survey_get_button_titles($button_title_result)
{

    //In diese Variable werden die Antwortmöglichkeiten für jede Frage eingefügt
    $button_title_array = array();

    //Diese Variable zählt die Durchgänge innerhalb der foreach-Schleife
    $iterations = 0;

    //Diese Variable zählt die Antwortmöglichkeiten jeder Antwort-ID
    $response_option_counter = 0;

    //Diese Variable dient zum Vergleich zweier Antwort-IDs
    $answer_id = 0;

    //Diese Variable zählt die Anzahl der Antworten
    $answer_id_counter = 0;

    //Es wird über alle Antwortmöglichkeiten iteriert
    foreach ($button_title_result as $button_title) {

        //Sollte es mindestens der zweite Durchlauf sein:
        if ($iterations > 0) {

            //Sollte die ID der aktuellen Antwort mit der vorigen übereinstimmen:
            if ($button_title->answer_id == $answer_id) {

                //Die Antwortmöglichkeit wird um eins erhöht und eingefügt
                $response_option_counter++;
                $button_title_array[$answer_id_counter][$response_option_counter] = $button_title->title;

            } else {
                //Sollte die ID der aktuellen Antwort nicht mit der vorigen übereinstimmen:
                $response_option_counter++;
                $button_title_array[$answer_id_counter][$response_option_counter] = "weiß nicht / keine Angabe";
                $response_option_counter = 0;
                $answer_id_counter++;
                $button_title_array[$answer_id_counter][$response_option_counter] = $button_title->title;
                $answer_id = $button_title->answer_id;

            }
        } else {

            //Im ersten Durchlauf wird das Array mit der ersten Antwort-ID und der ersten Antwortmöglichkeit befüllt.
            $button_title_array[$answer_id_counter][$response_option_counter] = $button_title->title;
            $answer_id = $button_title->answer_id;
        }
        $iterations++;
    }

    $response_option_counter++;
    $button_title_array[$answer_id_counter][$response_option_counter] = "weiß nicht / keine Angabe";


    return $button_title_array;
}

/**
 * Returns all user comments for a given question.
 * Does not display the user's own comment.
 *
 * @param $questionId
 *   ID of the question
 *
 * @return array
 *   All comments associated with the question
 */
function _survey_getComments($questionId, $answerId, $userId) {
  $commentsResult = db_query("SELECT answer, comment FROM {question_user_answers} 
        WHERE question_id = :question_id AND answer_id = :answer_id AND is_last_answer = 1
        AND user_pw != :user_id AND comment != '' ORDER BY answer DESC", array(
    ':question_id' => $questionId,
    ':answer_id' => $answerId,
    ':user_id' => $userId,
  ));

  $allComments = array();
  foreach ($commentsResult as $comment) {
    if ($comment->comment) {
      $allComments[$comment->answer][] = $comment->comment;
    }
  }

  return $allComments;
}

/**
 * Adds a toggleable user comment section to the ansers.
 *
 * @param $form
 *    The form array.
 * @param $i
 *    The current index.
 * @param $question_id
 *    The ID of the current question site.
 * @param $answer_id
 *    The ID of the answer.
 *
 * @return array
 *    The extended form array.
 */
function _survey_addUserComments($form, $i, $question_id, $answer_id, $user_id, $answerLabels) {
  $userComments = _survey_getComments($question_id, $answer_id, $user_id);

  if (count($userComments) > 0) {
    $form['fieldset' . $i]['toggleComments' . $i] = array(
      '#type' => 'checkbox',
      '#title' => t('Alle Kommentare anzeigen'),
      '#prefix' => '<div class="toggleComments">',
      '#suffix' => '</div>',
      '#attributes' => array(
        'class' => array('button')
      ),
    );

    $form['fieldset' . $i]['allComments' . $i] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('form-item')),
      '#states' => array(
        // Ausblenden, wenn "alle Kommentare anzeigen" nicht ausgewählt ist
        'invisible' => array(
          ':input[name="toggleComments' . $i . '"]' => array('checked' => FALSE),
        )),
    );

    $orderedComments = array();
    $answerLabel = '';


    // convert associative array with answers and comments to strings
    foreach ($userComments as $answer => $comments) {

      if (empty($answerLabels)) {
        // int textfield answer
        if ($answer === 'answer_NA') {
          $answerLabel = t('weiß nicht / keine Angabe');
        } else {
          $answerLabel = $answer;
        }
      } else {
        $answerLabel = $answerLabels[$answer];
      }

      foreach ($comments as $comment) {
        $orderedComments[] = '['.$answerLabel.'] ' . $comment;
      }
    }

    $form['fieldset' . $i]['allComments' . $i]['comments'] = array(
      '#theme' => 'item_list',
      '#list_type' => 'ul',
      '#items' => $orderedComments,
    );
  }

  return $form;
}
