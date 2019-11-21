<?php
/**
 * Created by PhpStorm.
 * User: jonaskortum
 * Date: 24.07.17
 * Time: 11:35
 */

/*
 * Diese Funktion erstellt ein Formular, in das eine Frage bearbeitet werden kann.
 */
function delphi_question_change_question_form()
{

  //Die ID der entsprechenden Frage
    $question_id = arg(1);

    $number_of_answers = arg(2);
    $question_title = "";
    $pageType = "";

    //Die entsprechende Frage wird aus der DB geladen
    $question_result = db_query("SELECT * FROM {question} WHERE question_id = :question_id", array(
        ':question_id' => $question_id
    ));

    // Eigenschaften der Frage werden ermittelt
    foreach ($question_result as $question) {
        $question_title = $question->title;
        $pageType = $question->type;
    }

    $title = "Titel der These";
    if ($pageType === 'group') {
        $title = "Titel des Themenbereichs";
        $form['#attributes']['class'][] = 'question-group-form';
      }

    //Textfeld, in das der Titel der Frage eingesetzt wird
    $form['question'] = array(
        '#type' => 'textarea',
        '#required' => TRUE,
        '#title' => $title,
        '#resizable' => FALSE,
        '#rows' => 2,
        '#default_value' => $question_title,
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
        '#default_value' => $number_of_answers,
        '#element_validate' => array('element_validate_integer_positive'),

    );

    //Button, der die Änderung der Anzahl von Antwortmöglichkeiten übernimmt
    $form['back'] = array(
        '#type' => 'submit',
        '#value' => 'Ändern',
        '#limit_validation_errors' => array(array('quantity')),
        '#submit' => array('delphi_question_change_question_change_quantity')
    );

    $answer_array = array();
    $check_array = array();
    $answer_id_array = array();
    $allQuestionsOnPage = array();
    $answer_counter = 0;

    //Die Antwortmöglichkeiten der Frage werden ermittelt
    $question_result = db_query("SELECT * FROM {question_possible_answers} WHERE question_id = :question_id ORDER BY question_id, weight, answers_id", array(
        ':question_id' => $question_id
    ));

    foreach ($question_result as $item) {
        array_push($answer_array, $item->description);
        array_push($check_array, $item->isRadioButton);
        array_push($answer_id_array, $item->answers_id);
        $allQuestionsOnPage[] = $item;
        $answer_counter++;
    }

    //Damit die nachfolgende Schleife funktioniert, muss das answer-array mindestens 6 groß sein
    while (sizeof($answer_array) < 6) {
        array_push($answer_array, "");
    }

    $answerSets = delphi_question_get_answer_sets();

    //Diese Schleife wird so oft durchgeführt, wie viele Antwortmöglichkeiten es geben soll
    for ($i = 1; $i <= $number_of_answers; $i++) {

      // TODO these should be all available question types
      $answer_choose_array = array('rating' => t('Radio-Buttons'),
                                   'year' => "Year",
                                   'text' => t('Text'),
                                   //'knowledgeMap' => t('Knowledge Map')
      );

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
            '#default_value' => $answer_array[$i - 1],
            '#required' => TRUE,
            '#maxlength' => 1000,
            '#attributes' => array(
              'class' => array('item-title'),
            ),
        );

        //Für jede Antwortmöglichkeit muss bestimmt werden, ob die Antwort per Radio-Buttons oder per Textfeld
        //gegeben werden soll
        $form['content' . $i]['radios' . $i] = array(
            '#type' => 'radios',
            '#title' => 'Art der Antwortmöglichkeit:',
            '#default_value' => $allQuestionsOnPage[$i - 1]->question_type,
            '#options' => $answer_choose_array,
        );

        $button_counter_result = 0;
        $button_title_array = array();

        //Wenn es sich bei der Antwortmöglichkeit um Radio-Buttons handelt
        if ($check_array[$i - 1]) {

            //Die Titel der Buttons werden geladen...
            $button_result = db_query("SELECT * FROM {question_buttons_title} WHERE answer_id = :answer_id", array(
                ':answer_id' => $answer_id_array[$i - 1]
            ));

            //... und abgespeichert
            foreach ($button_result as $item_button_result) {
                $button_counter_result = $button_counter_result + 1;
                array_push($button_title_array, $item_button_result->title);
            }

            while (sizeof($button_title_array) < 6) {
                array_push($button_title_array, "Dimension");
            }
        } else {
            array_push($button_title_array, "Dimension");
            array_push($button_title_array, "Dimension");
            array_push($button_title_array, "Dimension");
            array_push($button_title_array, "Dimension");
            array_push($button_title_array, "Dimension");
            array_push($button_title_array, "Dimension");
            array_push($button_title_array, "Dimension");
            $button_counter_result = 4;
        }

        $button_array = array(2 => "2", 4 => "4", 5 => "5", 6 => "6");
        $var = $i;

        //Sollte es sich bei der Antwortmöglichkeit um Radio-Buttons handeln, muss festgelegt werden, viele viele
        //Buttons für die Antwort vorgesehen sind.
        $form['content' . $i]['button_radios' . $i] = array(
            '#type' => 'radios',
            '#title' => "<b>Anzahl der vorhandenen Radio-Boxen</b>",
            '#default_value' => $button_array[$button_counter_result],
            '#options' => $button_array,
            '#states' => array(
                'visible' => array(
                    ':input[name="radios' . $var . '"]' => array('value' => 'rating'),
                ),
            ),

        );

      // Auswahlliste mit bereits verwendeten Antwort-Sets
      if (!empty($answerSets)) {
          $selectOptions = array();

          foreach ($answerSets as $key => $set) {
              //dsm($set);
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

        $form['content' . $i]['textfield_first_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der ersten Radio-Box",
            '#default_value' => $button_title_array[0],
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

        $form['content' . $i]['textfield_second_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der zweiten Radio-Box",
            '#default_value' => $button_title_array[1],
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

        $form['content' . $i]['textfield_third_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der dritten Radio-Box",
            '#default_value' => $button_title_array[2],
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

        $form['content' . $i]['textfield_fourth_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der vierten Radio-Box",
            '#default_value' => $button_title_array[3],
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

        $form['content' . $i]['textfield_fiveth_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der fünften Radio-Box",
            '#default_value' => $button_title_array[4],
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

        $form['content' . $i]['textfield_sixth_button' . $i] = array(
            '#type' => 'textfield',
            '#require' => TRUE,
            '#title' => "Bezeichnung der sechsten Radio-Box",
            '#default_value' => $button_title_array[5],
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

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Speichern',
        '#id' => 'submit',
        '#submit' => array('delphi_question_change_question_save_changes')
    );

    return $form;
}


/*
 * Diese Funktion speichert die Änderungen an eine Frage in die Datenbank ab.
 */
function delphi_question_change_question_save_changes($form, &$form_state)
{

    //Der Titel der Frage wird geladen
    $title = $form_state['values']['question'];

    //ID der Frage wird geladen
    $question_id = arg(1);

    //Anzahl Antwortmöglichkeiten werden geladen
    $quantity_id = arg(2);

    db_update('question')
        ->fields(array(
            'title' => $title,
        ))
        ->condition('question_id', $question_id, '=')
        ->execute();

    //Die alten Antwortmöglichkeiten werden gelöscht
    db_query("DELETE FROM {question_possible_answers} WHERE question_id = :question_id", array(
        ':question_id' => $question_id
    ));

    //Die Titel der gelöschten Buttons werden gelöscht
    db_query("DELETE FROM {question_buttons_title} WHERE question_id = :question_id", array(
        ':question_id' => $question_id
    ));

    //Es wird über die Anzahl Antwortmöglichkeiten iteriert
    for ($i = 1; $i <= $quantity_id; $i++) {

        //Frage wird ausgelesen
        $description = $form_state['values']['test' . $i];

        //Check, ob Antwort Radio-Buttons oder nicht
        $questionType = $form_state['values']['radios' . $i];
        $isRadioButton = 1;

        // TODO function to check for the question type
        //Wenn es sich nicht um Radio-Buttons handelt...
        if ($questionType !== 'rating') {
            $isRadioButton = 0;
        }

        //Antwort wird eingespeichert
        $answer_id = db_insert('question_possible_answers')
            ->fields(array(
                'description' => $description,
                'question_type' => $questionType,
                'isRadioButton' => $isRadioButton,
                'question_id' => $question_id,
                'weight' => $i,
            ))
            ->execute();

        //Wenn es sich um Radio-Buttons handelt
        if ($questionType === 'rating') {

            $var = $form_state['values']['button_radios' . $i];

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

            //Es wird über alle Titel iteriert
            for ($j = 0; $j <= $var - 1; $j++) {


                $title = $radio_array[$j];

                //Die Titel werden abgespeichert
                db_insert('question_buttons_title')
                    ->fields(array(
                        'question_id' => $question_id,
                        'answer_id' => $answer_id,
                        'button_id' => $j,
                        'title' => $title,
                    ))
                    ->execute();
            }
        }
    }

    drupal_goto("question_overview");

}


function delphi_question_change_question_change_quantity($form, &$form_state)
{
    $question_id = arg(1);
    $quantity_id = arg(2);
    $quantity = $form_state['values']['quantity'];


    if ($quantity_id == $quantity) {
        //do nothing
    } else if ($quantity > $quantity_id) {
        $diff = $quantity - $quantity_id;
        for ($i = 0; $i < $diff; $i++) {

            $answer_id = db_insert('question_possible_answers')
                ->fields(array(
                    'description' => "Neue Antwortmöglichkeit",
                    'isRadioButton' => 1,
                    'question_type' => 'text',
                    'question_id' => $question_id,
                    'weight' => $quantity + 1,
                ))
                ->execute();

            for ($j = 0; $j < 4; $j++) {
                db_insert('question_buttons_title')
                    ->fields(array(
                        'question_id' => $question_id,
                        'answer_id' => $answer_id,
                        'button_id' => $j,
                        'title' => "Neue Button-Bezeichnung",
                    ))
                    ->execute();
            }
        }
        drupal_goto("change_question/" . $question_id . "/" . $quantity);
    } else if ($quantity < $quantity_id) {
        $diff = $quantity_id - $quantity;

        for ($i = 0; $i < $diff; $i++) {

            $sql = "SELECT * FROM {question_possible_answers} WHERE question_id = " . $question_id . " ORDER BY answers_id DESC LIMIT 1";
            $result = db_query($sql);


            foreach ($result as $item) {
                $sql2 = "DELETE FROM {question_buttons_title} WHERE answer_id = " . $item->answers_id;
                db_query($sql2);
            }

            $sql = "DELETE FROM {question_possible_answers} WHERE answers_id = " . $item->answers_id;
            db_query($sql);
        }
        drupal_goto("change_question/" . $question_id . "/" . $quantity);

    }
}