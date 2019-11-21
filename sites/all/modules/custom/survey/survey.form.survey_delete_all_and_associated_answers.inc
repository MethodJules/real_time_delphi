<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 16.11.18
 * Time: 17:19
 */

/**
 * Confirmation form to delete all survey participants / tokens and their saved answers.
 */
function survey_tokens_delete_form($form, &$form_state) {

    $form = confirm_form($form,
        t('Alle Zugangsschlüssel inklusive abgegebener Antworten löschen?'),
        'survey/tokens',
        t('This action cannot be undone.'),
        t('Delete'),
        t('Cancel')
    );

    return $form;
}

/**
 * Deletes all survey tokens and saved answers.
 */
function survey_tokens_delete_form_submit($form, &$form_state) {
    db_truncate('question_user_answers')->execute();
    db_truncate('survey_users')->execute();

    $form_state['redirect'] = 'survey/tokens';

    drupal_set_message(t('Alle Zugangsschlüssel inklusive abgegebener Antworten gelöscht.'), 'status');
}
