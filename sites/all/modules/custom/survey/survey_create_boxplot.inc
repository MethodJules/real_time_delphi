<?php
/**
 * Created by PhpStorm.
 * User: jonaskortum
 * Date: 24.07.17
 * Time: 12:36
 */

//Create boxplot
function get_graphic($id, $isRadioButton, $indexNoAnswer)
{

    //Add js
    //drupal_add_js(drupal_get_path('module', 'survey') . '/js/survey_create_boxplot.js', 'footer');
    //drupal_add_css(drupal_get_path('module', 'survey') . '/css/survey_create_boxplot.css');

    $boxplotNoData = '<div class = "boxplot-nodata">' .
      t('Zu diesem Thema wurden noch nicht genügend Einschätzungen abgegeben.') .
      '</div>';

    $question_id = arg(1);
    $user_pw = arg(2);

    $testArrayOverall = array();
    $testArrayOverall2 = array();

    $quantity_check_result = db_query("SELECT * FROM {question_user_answers} WHERE question_id = :question_id AND answer_id = :answer_id AND is_last_answer = 1", array(
        ':question_id' => $question_id,
        ':answer_id' => $id
    ));

    $thresholds = array(
        0 => 1,
        1 => 2,
        2 => 3,
        3 => 4,
        4 => 5,
      );
    $threshold = $thresholds[variable_get('delphi_threshold')];

    //Sollten es mindestens fünf Antworten geben, kann der Boxplot erstellt werden
    if ($quantity_check_result->rowCount() > $threshold) { //TODO Change to config

        foreach ($quantity_check_result as $question_user_answer) {

            //Die Antwort des aktuellen Nutzers wird nicht eingerechnet
            if (strcmp($user_pw, $question_user_answer->user_pw)) {


                $user = $question_user_answer->user_pw;
                $check = survey_create_boxplot_search_for_user_ID($user, $testArrayOverall2);

                if ($check > -1) {
                    unset($testArrayOverall2[$check]);
                    unset($testArrayOverall[$check]);
                }
                array_push($testArrayOverall2, $question_user_answer);

                // "keine Ahnung" nicht als Antwort für Boxplot Berechnungen nutzen
                if ($question_user_answer->answer != $indexNoAnswer && $question_user_answer->answer != 'answer_NA') {
                    array_push($testArrayOverall, $question_user_answer->answer);
                }
            }
        }

        // corner case if ALL given answers are 'keine Angabe', thus no boxplot can be plotted.
        if (empty($testArrayOverall)) {
          return render($boxplotNoData);
        }

        //Die Antworten werden der Größe nach geordnet
        sort($testArrayOverall);

        //Wenn es sich bei der Antwortmöglichkeit um Radio-Buttons handelt, kann ein Boxplot erstellt werden
        if ($isRadioButton) {

            //Die einzelnen Bestandteile eines Boxplots werden ermittelt
            $min = 0;
            // Maximum der Boxplot-Skala, k.A. nicht Teil der Skala
            $max = $indexNoAnswer - 1;
            // TODO Berechnungfunktionen prüfen und refaktorieren, unnötige Unterscheidung, die nur wieder +1 addiert
            $avg = survey_get_average($testArrayOverall, $isRadioButton, $max);
            $median = survey_get_median($testArrayOverall, $isRadioButton, $max);
            $first_quantil = survey_get_first_quantil($testArrayOverall, $isRadioButton, $max);
            $third_quantil = survey_get_third_quantil($testArrayOverall, $isRadioButton, $max);
            $third_quantil2 = $third_quantil["procent"] - $first_quantil["procent"];

            //Für freie Antworten muss ein eigener Boxplot erstellt werden
        } else {

            $max = $testArrayOverall[count($testArrayOverall)-1];
            $avg = survey_get_average($testArrayOverall, $isRadioButton, $max);
            $median = survey_get_median($testArrayOverall, $isRadioButton, $max);
            $first_quantil = survey_get_first_quantil($testArrayOverall, $isRadioButton, $max);
            $third_quantil = survey_get_third_quantil($testArrayOverall, $isRadioButton, $max);
            $min = array_shift($testArrayOverall);
            $third_quantil2 = $third_quantil["procent"] - $first_quantil["procent"];
        }


        $links = array(0 => "unteres Quartil", 1 => "Mittelwert", 2 => "Median", 3 => "oberes Quartil");

        $boxplot_string = array(
            'container' => array(
                '#prefix' => '<div id="eins">',
                '#suffix' => '</div>',
                'boxplot' => array(
                    '#prefix' => '<div class="boxplot">',
                    '#suffix' => '</div>',
                    'boxlinie' => array(
                        '#prefix' => '<div class="box linie">',
                        '#suffix' => '</div>',
                    ),
                    'boxwhisker' => array(
                        '#prefix' => '<div class="box whisker">',
                        '#suffix' => '</div>',
                    ),
                    'boxinterquant' => array(
                        '#prefix' => '<div class="box interquart" style="left: ' . $first_quantil["procent"] . '%;">',
                        '#suffix' => '</div>',
                    ),
                    'boxthirdquant' => array(
                        '#prefix' => '<div class="box thirdquant" style="left: calc(' . $third_quantil2 . '% + ' . $first_quantil["procent"] . '%); width: ' . $third_quantil2 . '%; margin-left: -' . $third_quantil2 . '%;">',
                        '#suffix' => '</div>',
                    ),
                    'boxmedian' => array(
                        '#prefix' => '<div class="box median" style="left: ' . $median["procent"] . '%;">',
                        '#suffix' => '</div>',
                    ),
                    'boxmittel' => array(
                        '#prefix' => '<div class="box mittel" style="left: ' . $avg["procent"] . '%;">',
                        '#suffix' => '</div>',
                    ),
                    's_min' => array(
                        '#prefix' => '<span class="schild s_min" style="left: 0%;">',
                        '#suffix' => '</span>',
                        'markup' => array(
                            //'#markup' => _formatNumber($min+1),
                            '#markup' => 'min(' . _formatNumber($min + 1) . ')',
                        ),
                    ),
                    's_average' => array(
                        '#prefix' => '<span class="schild min s_average" style="margin-left: ' . $avg["procent"] . '%;">',
                        '#suffix' => '</span>',
                        'markup' => array(
                            '#markup' => _formatNumber($avg["absolut"] + 1),
                        ),
                    ),
                    's_median' => array(
                        '#prefix' => '<span class="schild min s_median" style="margin-left: ' . $median["procent"] . '%;"> ',
                        '#suffix' => '</span>',
                        'markup' => array(
                            '#markup' => _formatNumber($median["absolut"] + 1),
                        ),
                    ),
                    's_third_quantil' => array(
                        '#prefix' => '<span class="schild min s_third_quantil" style="margin-left: ' . $third_quantil["procent"] . '%;"> ',
                        '#suffix' => '</span>',
                        'markup' => array(
                            '#markup' => _formatNumber($third_quantil["absolut"] + 1),
                        ),
                    ),
                    's_first_quantil' => array(
                        '#prefix' => '<span class="schild s_first_quantil" style="margin-left: ' . $first_quantil["procent"] . '%;"> ',
                        '#suffix' => '</span>',
                        'markup' => array(
                            '#markup' => _formatNumber($first_quantil["absolut"] + 1),
                        ),
                    ),
                    's_max' => array(
                        '#prefix' => '<span class="schild min s_max" style="margin-left: 100%;"> ',
                        '#suffix' => '</span>',
                        'markup' => array(
                            //'#markup' => _formatNumber($max+1),
                            '#markup' => 'max(' . _formatNumber($max+1) . ')'
                        ),
                    ),
                ),
                'legend' => array(
                    '#theme' => 'item_list',
                    '#items' => $links,
                    '#type' => 'ul',
                    '#prefix' => '<div class="legend">',
                    '#suffix' => '</div>',
                    '#attributes' => array('class' => 'my-list'),
                ),
                'tooltip' => array(
                    '#prefix' => '<div class="tooltip"><span>Infos zum Boxplot</span><div class="boxplot_description">',
                    '#suffix' => '</div></div>',
                    '#theme' => 'table',
                    '#header' => array('Kennzahl', 'Beschreibung'),
                    '#rows' => array(
                        array('Unteres Quartil', 'Die kleinsten 25 % der Datenwerte sind kleiner als dieser oder gleich diesem Kennwert'),
                        array('Median', 'Die kleinsten 50 % der Datenwerte sind kleiner als dieser oder gleich diesem Kennwert'),
                        array('Oberes Quartil', 'Die kleinsten 75 % der Datenwerte sind kleiner als dieser oder gleich diesem Kennwert'),
                        array('Maximum', 'Größter Datenwert des Datensatzes')
                    ), 
                ),
            ),


        );

    } else {
        $boxplot_string = $boxplotNoData;
    }


    return render($boxplot_string);
}

/*
 * Diese Funktion rundet einen Wert auf eine ganze Zahl
 */
function survey_create_boxplot_round_to_valid_value($value)
{

    $tempzahl = explode(".", $value);

    if (isset($tempzahl[1])) {

        $value = round($value, 1);
        $tempzahl = explode(".", $value);


        switch ($tempzahl[1]) {
            case $tempzahl[1] >= 0 && $tempzahl[1] < 3:
                $tempzahl[1] = 0;
                break;
            case $tempzahl[1] >= 3 && $tempzahl[1] < 7:
                $tempzahl[1] = 5;
                break;
            case $tempzahl[1] >= 7:
                $tempzahl[0] = $tempzahl[0] + 1;
                $tempzahl[1] = 0;
                break;
            default:
                break;
        }

        if ($tempzahl[1] == 0) {
            $zahl = $tempzahl[0];
        } else {

            if ($tempzahl[0] != 0) {
                $zahl = implode(".", $tempzahl);
            } else {
                $zahl = 1;
            }
        }

    } else {
        $zahl = $value;
    }
    return $zahl;
}

/*
 * Diese Funktion überprüft, ob das User-PW valide ist
 */
function survey_create_boxplot_search_for_user_ID($id, $array)
{

    $check = -1;

    foreach ($array as $key => $val) {
        if (!strcmp($val->user_pw, $id)) {
            $check = $key;
        }
    }
    return $check;
}


function getQuantil($array)
{
    $returnArray = array();
    $temp = sizeof($array) / 2;
    if (is_int($temp)) {
        $returnArray["number"] = $temp;
        $returnArray["check"] = true;

    } else {
        $temp = round($temp, 0, PHP_ROUND_HALF_DOWN);
        $returnArray["number"] = $temp;
        $returnArray["check"] = false;
    }

    return $returnArray;
}

/**
 * Formatiert Rationale Zahlen auf eine Nachkommastelle mit Komma als Dezimaltrennzeichen. Ganze Zahlen werden ohne
 * Nachkommastellen ausgegeben.
 *
 * @param number|string $number
 *   Unformatierte Zahl
 *
 * @return number|string
 *   Formatierte Zahl
 */
function _formatNumber($number) {
  if (is_numeric($number) && floor($number) != $number) {
    $number = number_format($number,1, ",", "");
  } else {
    $number = number_format($number,0);
  }

  return $number;
}
