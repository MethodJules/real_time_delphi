<?php
/**
 * Created by PhpStorm.
 * User: jonaskortum
 * Date: 10.05.17
 * Time: 18:18
 */

/*
 * Diese Funktion baut ein Formular, in das der Nutzer Antworten zu einer entsprechenden Frage eingeben kann.
 * Der Passcode des Nutzers und die Fragen-ID der Frage werden über die URL übergeben.
 */
function survey_form()
{

    //Die ID der Frage wird aus der URL ausgelesen
    $question_id = arg(1);

    //Der Passcode des Nutzers wird aus der URL ausgelesen
    $user_pass = arg(2);

    //Das Form-Element, welches am Ende zurückgegeben wird, wird erstellt
    $form = array();

    //Die bisherigen Antworten des Nutzers auf die Frage werden aus der Datenbank geladen
    $user_answers = survey_get_answers($question_id, $user_pass);

    // Weiter zur Überprüfungsseite, wenn der Nutzer die Fragen bereits beantwortet hat

    $questionType = _survey_get_question_type($question_id);

    if (!empty($user_answers) && $questionType !== 'group') {
      drupal_goto("survey_question_evaluation/" . $question_id . "/" . $user_pass);
    }

    //Es werden alle verfügbaren Fragen aus der Datenbank geladen und gezählt
    $questions = survey_get_all_questions();
    $number_of_questions = false;
    $question_index = 0;
    $questionGroupIndex = 0;

    foreach ($questions as $q) {
        if ($q->question_id == $question_id) {
            $number_of_questions = true;
            break;
        }

        if ($q->type === 'group') {
          $questionGroupIndex++;
        }
        $question_index++;
    }

    //Sollte es ein valider Passcode des Nutzers sein und die Frage ebenfalls valide sein
    if (survey_check_valid_user_pw($user_pass) && $number_of_questions) {


        //Der Titel der Frage wird geladen, die Überschrift wird erstellt
        $text = $questions[$question_index]->title;
        $questionType = $questions[$question_index]->type;

        if ($questionType === 'group') {
          $headline = "
            <h1>Themenbereich " . ($questionGroupIndex + 1) . "</h1>
            <h2>" . $text . "</h2>";
        } else {
          $headline = "
            <h1>These " . ($question_index - $questionGroupIndex + 1) . "</h1>
            <h2>" . $text . "</h2>";
        }


        //Die Antworten der Frage werden aus der Datenbank geladen
        $answers = survey_get_answers_from_question($questions[$question_index]->question_id);


        //Die Bezeichungen der Buttons bzw. der Textfelder werden aus der Datenbank geladen
        $result = db_query("SELECT * FROM {question_buttons_title} WHERE question_id = :question_id ORDER BY answer_id, button_id", array(
            ':question_id' => $questions[$question_index]->question_id
        ));

        //Die Bezeichungen der Buttons werden geladen
        $button_title_array = survey_get_button_titles($result);

            $form["question"]['title'] = array(
                '#type' => 'markup',
                '#title' => t('survey quesion title'),
                '#markup' => $headline,
                '#prefix' => '<div class="question-header">',
            );


        // Da Titel immer fix im Vordergrund steht muss die Content-Region um die Höhe des Titels verschoben werden
        $cssClassContent = 'question-content-wrapper';
        $titleLength = drupal_strlen($text);

        if ($titleLength > 300) {
          $form['question']['title']['#suffix'] = '</div><div class="four-rows ' . $cssClassContent . '">';
        } else if ($titleLength > 200) {
          $form['question']['title']['#suffix'] = '</div><div class="three-rows ' . $cssClassContent . '">';
        } else if ($titleLength > 100) {
          $form['question']['title']['#suffix'] = '</div><div class="two-rows ' . $cssClassContent . '">';
        } else {
          $form['question']['title']['#suffix'] = '</div><div class="one-row ' . $cssClassContent . '">';
        }


        //Es wird über jede Antwortmöglichkeit iteriert
        $countTextfield = 0;
        for ($i = 0; $i < sizeof($answers); $i++) {

            //Sollte es bereits eine Antwort vom Nutzer in der Datenbank geben, wird diese als Standardwert
            //eingetragen
            if (!isset($user_answers[$i]->answer)) {
                $default_value = "";
            } else {
                $default_value = $user_answers[$i]->answer;
            }

            //Sollte es sich bei der Antwortmöglichkeit um Radio-Buttons handeln
            $questionType = $answers[$i]->question_type;
            if ($answers[$i]->isRadioButton) {
                $form['dim' . $i] = array(
                    '#type' => 'radios',
                    '#prefix' => '<div id ="visualization' . $i . '" class = "answers">',
                    '#suffix' => '</div>',
                    '#title' => $answers[$i]->description,
                    '#required' => true,
                  // TODO quick fix
                  // $button_title_array enthält eine Eintrag pro Radio-Button Antwortmöglichkeit. Textfeld
                  // Antwortmöglichkeiten werden außer acht gelassen. Bei 5 Antwortmöglichkeiten von denen 3 Textfelder
                  // sind würde $answers 5 Eintrage haben, $button_title_array jedoch nur 2 (-> Undefined offset).
                  // $countTextfield soll dies kompensieren.
                    '#options' => $button_title_array[$i - $countTextfield],
                );

                // Antwort vorauswählen, wenn die Frage bereits beantwortet wurde
                if (isset($user_answers[$i])) {
                  $form['dim' . $i]['#default_value'] = $user_answers[$i]->answer;
                }

                //Sollte es sich bei der Antwortmöglichkeit um ein Textfeld handeln
            } else if ($questionType === 'year') {
                $countTextfield++;

                // 'answer_NA' ist interne Codierung für 'keine Angabe' und soll nicht angezeigt werden.
                // TODO Typ DB-Feld für Antworten ändern?!
                $isChecked = FALSE;
                if ($default_value === 'answer_NA') {
                  $default_value = '';
                  $isChecked = TRUE;
                }

                $form['answer' . $i] = array(
                    '#type' => 'textfield',
                    '#prefix' => '<div id ="visualization' . $i . '" class = "answers">',
                    '#title' => $answers[$i]->description,
                    '#size' => 4,
                    '#maxlength' => 4,
                    '#default_value' => $default_value,
                    '#required' => FALSE,
                    '#states'        => array(
                      'disabled' => array(
                        ':input[name="answer_check' . $i . '"]' => array('checked' => TRUE),
                      ),
                    ),
                    '#element_validate' => array('survey_textfield_validate'),
                );
                $form['answer_check' . $i] = array(
                  '#type' => 'checkbox',
                  '#title' => 'weiß nicht / keine Angabe',
                  '#attributes' => array(
                    'class' => array('text-checkbox')
                  ),
                  '#default_value' => $isChecked,
                  '#suffix' => '</div>',
                );

            } else if ($questionType === 'text') {
              $countTextfield++;

              // 'answer_NA' ist interne Codierung für 'keine Angabe' und soll nicht angezeigt werden.
              $isChecked = FALSE;
              if ($default_value === 'answer_NA') {
                $default_value = '';
                $isChecked = TRUE;
              }

              $divClasses = 'answers ' . $questionType;

              $form['answer' . $i] = array(
                '#type' => 'textarea',
                '#prefix' => '<div id ="visualization' . $i . '" class = "' . $divClasses .'">',
                '#title' => $answers[$i]->description,
                '#resizable' => FALSE,
                '#rows' => 2,
                '#default_value' => $default_value,
                '#required' => FALSE,
                '#states'        => array(
                  'disabled' => array(
                    ':input[name="answer_check' . $i . '"]' => array('checked' => TRUE),
                  ),
                ),
                '#element_validate' => array('_survey_textarea_validate'),
              );
              $form['answer_check' . $i] = array(
                '#type' => 'checkbox',
                '#title' => 'weiß nicht / keine Angabe',
                '#attributes' => array(
                  'class' => array('text-checkbox')
                ),
                '#default_value' => $isChecked,
                '#suffix' => '</div>',
              );
            } else if ($answers[$i]->question_type === 'knowledgeMap') {
              $countTextfield++;

              //TODO
            }
        }

        $form['content-wrapper-close'] = array(
          '#type' => 'markup',
          '#markup' => '</div>',
        );

        // Damit 'Enter' den Weiter-Button triggert, muss dieser zuerst angelegt werden. Sollten weitere Buttons
        // eingefügt werden, werden diese per CSS nach links verschoben
        $form['submit-wrapper-open'] = array(
          '#type' => 'markup',
          '#markup' => '<div class="buttons-submit">',
        );
        $form['submit'] = array(
          '#type' => 'submit',
          '#value' => 'Weiter',
          '#submit' => array('survey_get_evaluation'),
        );

        //Sollte es sich nicht um die erste Frage handeln
        if ($question_index != 0) {
            $form['back'] = array(
                '#type' => 'submit',
                '#value' => 'Zurück',
                '#submit' => array('survey_question_evaluation_get_back_to_last_question'),
                // skip form vaildation on back button
                '#limit_validation_errors' => array(),
            );
        }
        $form['submit-wrapper-close'] = array(
          '#type' => 'markup',
          '#markup' => '</div>',
        );

        // The survey progress bar
        $form['progress_bar'] = array(
          '#markup' => _survey_get_status_bar($user_pass),
        );
    }

    // Drupal error messages mess with the fixed quesstion headers. Unset position:fixed if error messages are visible.
    $form['#attached']['js'][] = array(
      'type' => 'inline',
      'data' => "jQuery(document).ready(function () {if (jQuery('.messages.error').length) {
          jQuery('.question-header').css('position', 'unset');
          jQuery('.question-content-wrapper').css('padding-top', 0);

          // quick fix to highlight survey item on missing values
          jQuery('.form-type-radios').has('.error').css('border-color', 'red');
        }
        });",
      'scope' => 'footer',
    );

    return $form;
}

/*
 * Diese Funktion bringt den Nutzer zurück zur letzten Frage
 */
function survey_get_back_to_start()
{

    $question_id = arg(1);
    $user_pw = arg(2);

    if ($question_id == 1) {
        drupal_goto("");
    } else {
        $question_id--;
        drupal_goto("survey_question_evaluation/" . $question_id . "/" . $user_pw);
    }
}

/*
 * Diese Funktion speichert die Antworten des Nutzers in die Datenbank und leitet ihn weiter zur Anzeige
 * der Boxplots
 */
function survey_get_evaluation($form, &$form_state)
{

    //Die ID der Frage wird aus der URL ausgelesen
    $question = arg(1);

    //Der Passcode des Nutzers wird aus der URL geladen
    $user_passcode = arg(2);

    //Es werden alle Fragen aus der Datenbank geladen
    $questions = survey_get_all_questions();
    $answers = survey_get_answers_from_question($question);
    if(empty($answers)) {
      drupal_set_message('Bitte überprüfen Sie Ihre URL.');
    }

    //Es wird über alle Antworten iteriert
    $noAnswers = count($answers);
    for ($i = 0; $i < $noAnswers; $i++) {

        if ($answers[$i]->isRadioButton) {
          $userAnswer = $form_state['values']['dim' . $i];
        } else {
          $userAnswer = $form_state['values']['answer' . $i];
        }

        //Sollten bereits Antworten vorliegen, werden sie aktualisiert. Sonst werden neue Einträge angelegt
        $result = db_query("SELECT * FROM {question_user_answers} WHERE answer_id = :answer_id AND question_id = :question_id AND user_pw = :user_pw and is_last_answer = 1", array(
            ':answer_id' => $i,
            ':question_id' => $question,
            ':user_pw' => $user_passcode
        ));

        if ($result->rowCount()) {
          // Es gibt bereits Antworten vom Benutzer. Geänderte Antworten speichern und Änderungen in der DB behalten.
          foreach ($result as $item) {
            // Nur wenn Benutzer seine Antwort ändert, diese als neue Antwort speichern. Vorherige Antwort markieren (is_last_answer)
            if (strcmp($userAnswer, $item->answer)) {
              db_query("UPDATE {question_user_answers} SET is_last_answer = 0 WHERE answer_id = :answer_id AND question_id = :question_id AND user_pw = :user_pw", array(
                ':answer_id'   => $i,
                ':question_id' => $question,
                ':user_pw'     => $user_passcode
              ));

              db_insert('question_user_answers')
                ->fields(array(
                  'question_id' => $question,
                  'answer_id' => $i,
                  'answer' => $userAnswer,
                  'user_pw' => $user_passcode,
                  'comment' => $item->comment,
                  'is_last_answer' => 1,
                ))
                ->execute();
            }
          }

        // es gibt noch keine Antworten, also nur speichern
        } else {
            db_insert('question_user_answers')
                ->fields(array(
                    'question_id' => $question,
                    'answer_id' => $i,
                    'answer' => $userAnswer,
                    'user_pw' => $user_passcode,
                    'comment' => "",
                    'is_last_answer' => 1,
                ))
                ->execute();
        }
    }

    // no evaluation for question group pages
    $questionType = _survey_get_question_type($question);
    if ($questionType === 'group') {
      $nextQuestionId = survey_question_evaluation_get_next_question($question);

      if ($nextQuestionId == -1) {
        drupal_goto("finish_survey/" . $user_passcode);
      } else {
        drupal_goto("survey_question/" . $nextQuestionId . "/" . $user_passcode);
      }
    } else {
      drupal_goto("survey_question_evaluation/" . $question . "/" . $user_passcode);
    }
}