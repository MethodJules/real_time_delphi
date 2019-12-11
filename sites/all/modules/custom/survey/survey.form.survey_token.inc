<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 16.11.18
 * Time: 17:17
 */

/**
 * Builds the form to manage survey participants / tokens.
 */
function survey_token_form($form, &$form_state) {
    if (empty($form_state['values']['token_quantity'])) {
        // form to create new tokens
        $form['token_create'] = array(
            '#type' => 'fieldset',
            '#title' => t('Zugangsschlüssel erstellen'),
            '#collapsible' => TRUE,
            '#collapsed' => FALSE,
        );

        $form['token_create']['token_quantity'] = array(
            '#type' => 'textfield',
            '#title' => "Zugangsschlüssel",
            '#title_display' => 'invisible',
            '#size' => 1,
            '#maxlength' => 3,
            '#element_validate' => array('element_validate_integer_positive'),
            '#field_suffix' => t('Zugangsschlüssel für Umfrageteilnehmer erstellen.'),
        );

        $form['token_create']['submit'] = array(
            '#type' => 'submit',
            '#value' => 'Zugangsschlüssel erstellen',
        );

        $tokens = _get_all_respondent_identifieres();
        if (count($tokens) > 0) {
            $form['token_delete'] = array(
                '#type' => 'fieldset',
                '#title' => t('Erstellte Zugangsschlüssel'),
                '#collapsible' => TRUE,
                '#collapsed' => FALSE,
            );

            $form['token_delete']['delete_all'] = array(
                '#type' => 'link',
                '#title' => t('Alle Zugangsschlüssel löschen'),
                '#href' => 'survey/tokens/delete',
                '#attributes' => array('class' => array('form-submit')),
            );

            // create table of all tokens with a delete link
            $rows = array();
            foreach ($tokens as $token) {
                $rows[] = array(
                    'token' => $token->user_pw,
                    'link' => survey_get_token_delete_link($token->user_pw)
                );
            }

            $form['token_delete']['tokensdel_table'] = array (
                '#theme' => 'table',
                '#header' => array('Zugangsschlüssel', ''),
                '#rows' => $rows,
                '#title' => t('Zugangsschlüssel löschen'),
            );
        }
    } else {
        // table with created tokens and full survey links.
        $noTokens = $form_state['values']['token_quantity'];
        $count = 1;

        $header = array(t('Zugangsschlüssel'), t('Umfrage-URL'));
        $rows = array();
        while ($count <= $noTokens) {
            $token = survey_create_token();
            $rows[] = array(
                'token' => $token,
                'link' => f($token)
            );
            $count++;
        }

        $form['tokens_table'] = array (
            '#theme' => 'table',
            '#header' => $header,
            '#rows' => $rows,
            '#title' => t('Zugangsschlüssel'),
        );

        $form['csv_submit'] = array(
            '#type' => 'submit',
            '#value' => t('Zugangsschlüssel als CSV exportieren'),
            '#submit' => array('_survey_token_csv_export_callback'),
        );
    }

    return $form;
}

function survey_token_form_submit($form, &$form_state) {
    $form_state['rebuild'] = true;
}