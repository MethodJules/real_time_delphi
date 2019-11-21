<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 22.11.18
 * Time: 10:57
 */

function delphi_evaluation_data() {

    //Hole Daten aus possible answers
    $result = db_select('question_possible_answers', 'qba')
        ->fields('qba')
        ->condition('question_type', 'rating', '=')
        ->execute()
        ->fetchAll();

    //dsm($result);

    $answers = delphi_evaluation_get_answers($result);


    $all_data = array();

    foreach ($answers as $answer) {
        //dsm($answer[0]->Antwort);
        //dsm($answer);
        //$data = array();
        $data = array();
        foreach ($answer as $answer_value) {
            array_push($data, $answer_value->Antwort);

        }
        $answers_count = array_count_values($data);
        //dsm($answers_count);
        //dsm(key($answers_count));



        array_push($all_data, array($answers_count));
    }


    //dsm($all_data);
    //$answers_count = array_count_values($data);

    $pie_data = array();
    $data = array_unique($data);
    foreach ($data as $datum) {
        array_push($pie_data, array(
            $datum , $answers_count[$datum]
        ));
    }



    $these = end($answer)->These;
    dsm($these);
    $chart = array(
        'id' => 'visualization',
        'type' => 'piechart',
        'rows' => $pie_data,

    );

    $output = "<h2>" . $these . "</h2>" . d3_draw($chart);

    return $output;

}

function delphi_evaluation_get_answers($result) {

    $answers = array();

    foreach ($result as $row) {
        //dsm($row);
        $answer = db_select('delphi_data', 'dd')
            ->fields('dd')
            ->condition('These', $row->description, '=')
            ->execute()
            ->fetchAll();


        //dsm($answer);
        array_push($answers, $answer);
    }

    return $answers;

}