<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 16.11.18
 * Time: 17:46
 */




function survey_delete_form($form, &$form_state) {
    $form = confirm_form($form,
        t('Do you really want to delete everything?'),
        'survey/config',
        t('This action can not be undone'),
        t('Delete'),
        t('Cancel')

        );


    return $form;
}
/**
 * Deletes the whole survey (including all associated data)
 */
function survey_delete_form_submit() {
    db_truncate('question')->execute();
    db_truncate('question_possible_answers')->execute();
    db_truncate('question_buttons_title')->execute();
    db_truncate('question_user_answers')->execute();
    db_truncate('survey')->execute();
    db_truncate('survey_users')->execute();

    drupal_set_message(t('Survey deleted.'), 'status');
}