<?php
/**
 * Created by PhpStorm.
 * User: jonaskortum
 * Date: 24.07.17
 * Time: 11:30
 */

// TODO refactor, fast identisch mit delphi_question_change_question.inc
function delphi_question_add_question_form()
{

    //Die Anzahl der Antwortmöglichkeiten wird geladen
    $answer_quantity_id = arg(1);
    $isQuestionGroup = FALSE;

    // Übersichtsseite für einen Fragenblock erzeugen. Wird intern wie eine Frage behandelt (enthält auch genau eine
    // Bewertungsskala zur Speicherung der Selbsteinschätzung), aber enthält weniger Elemente.
    if ($answer_quantity_id === 'group') {
      $isQuestionGroup = TRUE;
      $answer_quantity_id = 1;

      $form['#attributes']['class'][] = 'question-group-form';
    }

    //Der Form wird eine ID übergeben
    $form['#id'] = 'question-form';

    // TODO these should be all available question types
    // TODO these should be all available question type
    //Daten für das Array, welches die Radio-Buttons benutzen
    $answer_choose_array = array('rating' => t('Radio-Buttons'),
                                 'year' => "Year",
                                 'text' => t('Text'),
                                 'ranking' => t('Ranking'),
                                 //'knowledgeMap' => t('Knowledge Map')
    );

    $button_array = array(2 => "2", 4 => "4", 5 => "5", 6 => "6");


    //Textfeld, in das der Titel der Frage eingesetzt wird
    $form['question'] = array(
        '#type' => 'textarea',
        '#required' => FALSE,
        '#title' => "Titel des Themenbereichs",
        //'#format' => 'full_html',
        '#attributes' => array(
            'id' => 'question-field',
        ),
        '#maxlength' => 1000, // DB field type is varchar(1023)
    );

    //Textfeld, in das die Anzahl der Antwortmöglichkeiten eingegeben werden soll
    $form['quantity'] = array(
        '#type' => 'textfield',
        '#title' => "Anzahl der Antwortmöglichkeiten",
        '#size' => 2,
        '#maxlength' => 2,
        '#default_value' => $answer_quantity_id,
        '#element_validate' => array('element_validate_integer_positive'),

    );

    //Button, der die Änderung der Anzahl von Antwortmöglichkeiten übernimmt
    $form['back'] = array(
        '#type' => 'submit',
        '#value' => 'Ändern',
        '#limit_validation_errors' => array(array('quantity')),
        '#submit' => array('delphi_question_add_question_change_quantity')
    );


    $answerSets = delphi_question_get_answer_sets();

    //Diese Schleife wird so oft durchgeführt, wie viele Antwortmöglichkeiten es geben soll
    for ($i = 1; $i <= $answer_quantity_id; $i++) {

        //Für jede Antwortmöglichkeit wird ein Fieldset zur Übersichtlichkeit angelegt
        $form['content' . $i] = array(
            '#title' => $i . ". Antwortmöglichkeit",
            '#type' => 'fieldset',
            '#description' => ""
        );

        //Für jede Antwortmöglichkeit muss eine Bezeichnung festgelegt werden.
        $form['content' . $i]['test' . $i] = array(
            '#type' => 'textarea',
            '#rows' => 1,
            '#resizable' => FALSE,
            '#title' => "<b>Bezeichnung der Antwortmöglichkeit:</b>",
            '#default_value' => "",
            '#required' => TRUE,
            '#maxlength' => 1000,
            '#attributes' => array(
              'class' => array('item-title'),
            ),

        );

        $var = $i;

        //Sollte es sich bei der Antwortmöglichkeit um Radio-Buttons handeln, muss festgelegt werden, viele viele
        //Buttons für die Antwort vorgesehen sind.
        // Eine Fragengruppe hat immer nur eine Skala mit 5 Radio-Buttons zur Selbsteinschätzung.
        if ($isQuestionGroup) {
          $form['content' . $i]['radios' . $i] = array(
            '#type' => 'radios',
            '#title' => 'Art der Antwortmöglichkeit:',
            '#default_value' => 'rating',
            '#options' => $answer_choose_array,
          );

          $form['content' . $i]['button_radios' . $i] = array(
            '#type'          => 'radios',
            '#title'         => "<b>Anzahl der vorhandenen Radio-Boxen</b>",
            '#default_value' => 5,
            '#options'       => $button_array,
          );

        } else {

          //Für jede Antwortmöglichkeit muss bestimmt werden, ob die Antwort per Radio-Buttons oder per Textfeld
          //gegeben werden soll
          $form['content' . $i]['radios' . $i] = array(
            '#type' => 'radios',
            '#title' => 'Art der Antwortmöglichkeit:',
            '#default_value' => 'rating',
            '#options' => $answer_choose_array,
          );
          $form['content' . $i]['button_radios' . $i] = array(
            '#type'          => 'radios',
            '#title'         => "<b>Anzahl der vorhandenen Radio-Boxen</b>",
            '#default_value' => $button_array[4],
            '#options'       => $button_array,
            '#states'        => array(
              'visible' => array(
                ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
              ),
            ),
          );

          $form['content' . $i]['ranking' .$i] = array(
              '#type' => 'radios',
              '#title' => t('Anzahl der Ranking Items'),
              '#default_value' => 2,
              '#options' => array(2 => '2', 3 => '3', 5 => '5'),
              '#states' => array(
                  'visible' => array(
                      ':input[name="radios' . $var . '"]' => array('value' => 'ranking'),
                  ),
              ),
          );
        }

        // Auswahlliste mit bereits verwendeten Antwort-Sets
        if (!empty($answerSets)) {
          $selectOptions = array();

          foreach ($answerSets as $key => $set) {
            $selectOptions[] = count($set) . ' ' . implode(', ', $set) ;
          }

          $form['content' . $i]['answer_sets' . $i] = array(
            '#title' => t('Antwort-Set'),
            '#type' => 'select',
            '#options' => $selectOptions,
            '#empty_option' => t(''),
            '#states' => array(
              'visible' => array(
                ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
              ),
            ),
            '#attributes' => array(
              'class' => array('answer-sets'),
            ),
          );
        }

        $defaultValues = array_fill(0, 6, 'Dimension ' . $var);
        if ($isQuestionGroup) {
          $defaultValues = array(
            '... kenne ich mich überhaupt nicht aus.',
            '.',
            '.',
            '.',
            '... kenne ich mich sehr gut aus.',
            '.'
          );
        }

        //Textfelder für die ersten beiden Ranking Felder

        $form['content' . $i]['textfield_first_ranking' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der ersten Ranking-Box",
            '#default_value' => t('Rang 1'),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="ranking' . $var . '"]' => array(
                        array('value' => t('2')),
                        array('value' => t('3')),
                        array('value' => t('5')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'ranking'),
                ),
            ),
        );

        $form['content' . $i]['textfield_second_ranking' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der zweiten Ranking-Box",
            '#default_value' => t('Rang 2'),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="ranking' . $var . '"]' => array(
                        array('value' => t('2')),
                        array('value' => t('3')),
                        array('value' => t('5')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'ranking'),
                ),
            ),
        );

        //Textfelder für das dritte Ranking Felder

        $form['content' . $i]['textfield_third_ranking' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der drittten Ranking-Box",
            '#default_value' => t('Rang 3'),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="ranking' . $var . '"]' => array(
                        array('value' => t('3')),
                        array('value' => t('5')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'ranking'),
                ),
            ),
        );

        //Textfelder für die fünf Ranking Felder
        $form['content' . $i]['textfield_fourth_ranking' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der vierten Ranking-Box",
            '#default_value' => t('Rang 4'),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="ranking' . $var . '"]' => array(
                        array('value' => t('5')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'ranking'),
                ),
            ),
        );

        $form['content' . $i]['textfield_fifth_ranking' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der fünften Ranking-Box",
            '#default_value' => t('Rang 5'),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="ranking' . $var . '"]' => array(
                        array('value' => t('5')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'ranking'),
                ),
            ),
        );



        //Textfeld für den ersten Button
        $form['content' . $i]['textfield_first_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der ersten Radio-Box",
            '#default_value' => t($defaultValues[0]),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="button_radios' . $var . '"]' => array(
                        array('value' => t('2')),
                        array('value' => t('4')),
                        array('value' => t('5')),
                        array('value' => t('6')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
                ),
            ),
        );

        //Textfeld für den zweiten Button
        $form['content' . $i]['textfield_second_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der zweiten Radio-Box",
            '#default_value' => t($defaultValues[1]),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="button_radios' . $var . '"]' => array(
                        array('value' => t('2')),
                        array('value' => t('4')),
                        array('value' => t('5')),
                        array('value' => t('6')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
                ),
            ),
        );

        //Textfeld für den dritten Button
        $form['content' . $i]['textfield_third_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der dritten Radio-Box",
            '#default_value' => t($defaultValues[2]),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="button_radios' . $var . '"]' => array(
                        array('value' => t('4')),
                        array('value' => t('5')),
                        array('value' => t('6')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
                ),
            ),
        );

        //Textfeld für den vierten Button
        $form['content' . $i]['textfield_fourth_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der vierten Radio-Box",
            '#default_value' => t($defaultValues[3]),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="button_radios' . $var . '"]' => array(
                        array('value' => t('4')),
                        array('value' => t('5')),
                        array('value' => t('6')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
                ),
            ),
        );

        //Textfeld für den fünften Button
        $form['content' . $i]['textfield_fiveth_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der fünften Radio-Box",
            '#default_value' => t($defaultValues[4]),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="button_radios' . $var . '"]' => array(
                        array('value' => t('5')),
                        array('value' => t('6')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
                ),
            ),
        );

        //Textfeld für den sechsten Button
        $form['content' . $i]['textfield_sixth_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der sechsten Radio-Box",
            '#default_value' => t($defaultValues[5]),
            '#required' => TRUE,
            '#states' => array(
                'visible' => array(
                    ':input[name="button_radios' . $var . '"]' => array(
                        array('value' => t('6')),
                    ),
                    ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
                ),
            ),
        );
    }

    $form['#attached']['js'][] = array(
      'type' => 'setting',
      'data' => array('delphi_question' => array('sets' => $answerSets)),
    );

    $form['#attached']['js'][] = array(
      'type' => 'file',
      'data' => drupal_get_path('module', 'delphi_question') . '/js/selectsets.js',
    );

    // automatically resize textareas to content
    $form = delphi_question_add_autosize($form);

    //Submit-Button
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Speichern',
        '#submit' => array('delphi_question_add_question_save_question')
    );

    return $form;
}

//Diese Funktion ändert die Anzahl von Antwortmöglichkeiten in der Adresszeile und lädt das Formular zum Anlegen einer
//Frage neu.
function delphi_question_add_question_change_quantity($form, &$form_state)
{
    $quantity = $form_state['values']['quantity'];
    drupal_goto("add_questions/" . $quantity);
}

//Diese Funktion speichert die eingebenen Daten für die Frage in die DB ab.
function delphi_question_add_question_save_question($form, &$form_state)
{
    $question_title = $form_state['values']['question'];

    $isQuestionGroup = FALSE;
    $questionType = "";

    // Fragegruppen haben nur eine Antwortmöglichkeit
    if (arg(1) === 'group') {
      $number_of_answers = 1;
      $isQuestionGroup = TRUE;
      $questionType = 'group';
    } else {
      $number_of_answers = arg(1);
      $questionType = 'question';
    }

    $noQuestions = db_query("SELECT COUNT(*) FROM {question}")->fetchField();

    //Der Titel der Frage wird in die DB gespeichert, um eine Fragen-ID zu erhalten
    $nid = db_insert('question')
        ->fields(array(
            'title' => $question_title,
            'weight' => $noQuestions + 1,
            'type' => $questionType,
        ))
        ->execute();

    //Es wird über alle Antworten iteriert
    for ($i = 1; $i <= $number_of_answers; $i++) {

        $description = $form_state['values']['test' . $i];

        //Der Typ der Antwort wird ermittelt. Entweder Radio-Button oder Textfeld
        $questionType = $form_state['values']['radios' . $i];
        $isRadioButton = 1;

      // TODO function to check for the question type
        if ($questionType !== 'rating') {
            $isRadioButton = 0;
        }

        //Die Antwortmöglichkeit wird in die DB gespeichert
        $answer_id = db_insert('question_possible_answers')
            ->fields(array(
                'description' => $description,
                'question_type' => $questionType,
                'isRadioButton' => $isRadioButton,
                'question_id' => $nid,
                'weight' => $i,
            ))
            ->execute();

        //Sollte es sich bei der Antwortmöglichkeit um Radio-Buttons handeln
        if ($questionType === 'rating') {

            //Es wird ausgelesen, ob 4,5 oder 6 Radio-Buttons zur Antwort gehören
            $number_of_radio_buttons = $form_state['values']['button_radios' . $i];

            $radio_name_1 = $form_state['values']['textfield_first_button' . $i];
            $radio_name_2 = $form_state['values']['textfield_second_button' . $i];
            $radio_name_3 = $form_state['values']['textfield_third_button' . $i];
            $radio_name_4 = $form_state['values']['textfield_fourth_button' . $i];
            $radio_name_5 = $form_state['values']['textfield_fiveth_button' . $i];
            $radio_name_6 = $form_state['values']['textfield_sixth_button' . $i];

            $radio_array = array();
            array_push($radio_array, $radio_name_1);
            array_push($radio_array, $radio_name_2);
            array_push($radio_array, $radio_name_3);
            array_push($radio_array, $radio_name_4);
            array_push($radio_array, $radio_name_5);
            array_push($radio_array, $radio_name_6);

            //Es wird über alle nötigen Radio-Buttons iteriert
            for ($j = 0; $j <= $number_of_radio_buttons - 1; $j++) {

                $question_title = $radio_array[$j];

                //Der Titel jedes Radio-Buttons wird abgespeichert
                db_insert('question_buttons_title')
                    ->fields(array(
                        'question_id' => $nid,
                        'answer_id' => $answer_id,
                        'button_id' => $j,
                        'title' => $question_title,
                    ))
                    ->execute();
            }
        }
    }
    drupal_goto("question_overview");
}