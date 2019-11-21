<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 03.11.18
 * Time: 20:06
 */

function delphi_question_add_ranking_question_form($form, &$form_state) {
    $answer_quantity_id = arg(1);

    $form['#id'] = 'ranking-question-form';

    // the characters
    $red_dwarf = array(
        array(
            'name' => 'Dave Lister',
            'type' => 'Human',
            'rank' => 1,
        ),
        array(
            'name' => 'Arnold Rimmer',
            'type' => 'Hologram',
            'rank' => 2,
        ),
        array(
            'name' => 'Cat',
            'type' => 'Humanoid',
            'rank' => 3,
        ),
        array(
            'name' => 'Kryten',
            'type' => 'Android',
            'rank' => 4,
        ),
    );

    $form['ranking_thesis'] = array(
        '#prefix' => '<div id="ranking_thesis">',
        '#suffix' => '</div>',
        '#tree' => TRUE,
        '#theme' => 'form_table_theme_name'
    );

    foreach ($red_dwarf as $key => $characters) {
        $form['ranking_thesis'][$key]['name'] = array(
            '#type' => 'textfield',
            '#default_value' => $characters['name'],
        );
        $form['ranking_thesis'][$key]['type'] = array(
            '#type' => 'textfield',
            '#default_value' => $characters['type'],
        );
        $form['ranking_thesis'][$key]['weight'] = array(
            '#type' => 'textfield',
            '#default_value' => $characters['rank'],
            '#size' => 3,
            '#attributes' => array('class' => array('rank-weight')), // needed for table dragging
        );
    }

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
    );

    return $form;

}

//Theme the form
/**
 * Implements hook_theme
 */
function delphi_question_theme($existing, $type, $theme, $path) {
    return array(
        'form_table_theme_name' => array(
            'render element' => 'element',
        ),
    );
}

function theme_form_table_theme_name($vars) {
    $element = $vars['element'];
    drupal_add_tabledrag('form_id', 'order', 'sibling', 'rank-weight'); // needed for table dragging

    $header = array(
        'name' => t('Name'),
        'type' => t('Type'),
        'weight' => t('Rank'),
    );

    $rows = array();
    foreach (element_children($element) as $key) {
        $row = array();

        $row['data'] = array();
        foreach ($header as $fieldname => $title) {
            $row['data'][] = drupal_render($element[$key][$fieldname]);
            $row['class'] = array('draggable'); // needed for table dragging
        }
        $rows[] = $row;
    }

    return theme('table', array(
        'header' => $header,
        'rows' => $rows,
        'attributes' => array('id' => 'form_id'), // needed for table dragging
    ));
}