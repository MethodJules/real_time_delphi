<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 16.11.18
 * Time: 16:24
 */

//Diese Funktion baut die Startseite der Umfrage
function survey_start_form($form, &$form_state)
{
    $survey = _survey_get_survey(1);
    //log register
    watchdog('survey', 'Teilnehmer %token hat sich eingeloggt',$variables=array('%token' => arg(1)), $severity = WATCHDOG_INFO,$link= NULL);
    drupal_set_title(check_plain($survey['title']));

    $form['welcome'] = array(
        '#markup' => $survey['welcome_msg'],
    );


    // user token is null if an admin uses internal start survey
    $userToken = arg(1);
    if (!is_null($userToken)) {
        if(count(_survey_get_answered_questions($userToken)) > 0) {
            // get the ID of first unanswered question
            $questionId = _survey_get_first_open_question(arg(1));

            // -1 = all questions have been answered
            if ($questionId !== -1) {
                $form['continue'] = array(
                    '#type' => 'submit',
                    '#value' => t('Umfrage fortführen'),
                    '#submit' => array('survey_continue_survey')
                );

                $form_state['#continue_qid'] = $questionId;
                $form_state['#continue_uid'] = $userToken;
            }
        }
    }

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Umfrage starten',
        '#submit' => array('survey_start_survey')
    );

    return $form;
}