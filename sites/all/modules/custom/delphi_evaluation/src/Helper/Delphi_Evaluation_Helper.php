<?php
/**
 * Created by PhpStorm.
 * User: julien
 * Date: 09.03.18
 * Time: 15:06
 */

namespace Drupal\delphi_evaluation\Helper;


class Delphi_Evaluation_Helper
{

    /**
     * Gets the survey data from the database
     * @return array
     * @throws \Exception
     */
    public function delphi_evaluation_get_survey_data($sql, $flag) {
        //$sql = "SELECT user_pw, title, type, q.question_id, weight, answer_id, answer, comment, is_last_answer FROM {question} q
        //LEFT JOIN {question_user_answers} a ON q.question_id = a.question_id;";

        $result = db_query($sql);

        $result = $result->fetchAll();
        $header = array(
            'user_pw',
            'Thesengruppe',
            //'question_id',
            'These',
            //'answer',
            'Antwort',
            'Kommentar',
            'Letzte Antwort(1/0)',
        );

        if ($flag == 1) {
            foreach ($result as $row) {
                $rows[] = array(
                    $row->user_pw,
                    $row->title,
                    //$row->question_id,
                    $this->delphi_evaluation_get_item($row->question_id, $row->answer_id + 1),
                    //$row->answer,
                    //$this->_get_answer_id($row->question_id, $this->delphi_evaluation_get_item($row->question_id, $row->answer_id + 1)),
                    $this->delphi_evaluation_get_answer($row->question_id, $this->_get_answer_id($row->question_id, $this->delphi_evaluation_get_item($row->question_id, $row->answer_id + 1)),$row->answer, $row->user_pw),
                    $row->comment,
                    $row->is_last_answer,

                );
            }


        } else {
            foreach ($result as $row) {
                $rows[] = array(
                    $row->user_pw,
                    $row->title,
                    $row->item,
                    //$row->answer,
                    $this->delphi_evaluation_get_answer($row->question_id, $row->answer_id,$row->answer),

                    $row->comment,
                    $row->is_last_answer,

                );
            }
        }

        $result = array('header' => $header, 'rows' => $rows);
        //return a HTML Table

        $this->_write_to_database($result);

        return $result;
    }


    public function _write_to_database($result) {
        $rows = $result['rows'];

        db_truncate('delphi_data')->execute();

        $query = db_insert('delphi_data')->fields(array('user_pw', 'Thesengruppe', 'These', 'Antwort', 'comment', 'is_last_answer'));
        foreach ($rows as $row) {
            $query->values($row);
        }

        $query->execute();
        drupal_set_message('Daten in Datenbank geschrieben.');
    }


    /**
     * Get answer_id
     *
     */
    public function _get_answer_id($question_id, $description) {
        $result = db_select('question_possible_answers', 'qba')
            ->fields('qba')
            ->condition('question_id', $question_id)
            ->condition('description', $description)
            ->execute()
            ->fetchAssoc();

        return $result['answers_id'];
    }


    /**
     * Gets the answer from the database based on question_id and answer_id
     */

    public function delphi_evaluation_get_answer($question_id, $answer_id, $answer, $user_pw) {

        $result = db_select('question_possible_answers', 'qba')
            ->fields('qba')
            ->condition('question_id', $question_id, '=')
            ->execute()
            ->fetchAssoc();

        $question_type = $result['question_type'];
        if($question_type == 'rating') {

            $result = db_select('question_buttons_title', 'qbt')
                ->fields('qbt')
                ->condition('question_id', $question_id, '=')
                ->condition('answer_id', $answer_id, '=')
                ->condition('button_id', $answer, '=')
                ->execute()
                ->fetchAssoc();
            return $result['title'];
        } elseif ($question_type == 'year') {
            $result = db_select('question_user_answers', 'qba')
                ->fields('qba')
                ->condition('question_id', $question_id, '=')
                //->condition('answer_id', $answer_id, '=')
                ->condition('user_pw', $user_pw, '=')
                ->execute()
                ->fetchAssoc();
            //dsm($result);
            return $result['answer'];
        } elseif ($question_type == 'text') {
            $result = db_select('question_user_answers', 'qba')
                ->fields('qba')
                ->condition('question_id', $question_id, '=')
                //->condition('answer_id', $answer_id, '=')
                ->condition('user_pw', $user_pw, '=')
                ->execute()
                ->fetchAssoc();
            //dsm($result);
            return $result['answer'];
        }


    }

    /**
     * Gets the item from the database based on question_id and and weight
     * weight is the answer_id+1 in the parameter call
     * @param $question_id
     * @param $weight
     * @return \DatabaseStatementInterface
     */
    public function delphi_evaluation_get_item($question_id, $weight) {
        $sql = "SELECT description, question_id, weight FROM {question_possible_answers} item
                WHERE item.question_id = :question_id AND item.weight = :weight";

        $result = db_query($sql, array('question_id' => $question_id, 'weight' => $weight));
        $result = $result->fetchField(0);

        return $result;
    }














  /**
   * Joins all relevant survey data into a single array. One row for each answer.
   * (user_pw | q.question_id | type | q.weight | title | answer_id | item.description | answer | comment | is_last_answer)
   * Can be saved into a separate database @see delphi_evaluation_save_to_database()
   *
   * @return \DatabaseStatementInterface
   * @throws \Exception
   */
  public function delphi_evaluation_join_survey_data() {

    $sql_query = "SELECT user_pw, q.question_id, type, q.weight, title, answer_id, item.description, answer, comment, is_last_answer
        FROM {question} q
        LEFT JOIN {question_user_answers} a ON q.question_id = a.question_id
        LEFT JOIN {question_possible_answers} item ON item.question_id = q.question_id AND item.weight = answer_id + 1 
        ORDER BY user_pw, q.weight, answer_id";

    $result = db_query($sql_query);

    $result = $result->fetchAll();
    return $result;
  }


  /**
   * Returns all relevant survey data for a specific user.
   * (user_pw | q.question_id | type | q.weight | title | answer_id | item.description | answer | comment | is_last_answer)
   *
   * @param string $token
   *   The user token.
   *
   * @return \DatabaseStatementInterface
   * @throws \Exception
   */
  public function delphi_evaluation_get_user_data($token) {

    $sql_query = "SELECT u.user_pw, u.question_id, q.type, q.weight, q.title, u.answer_id, a.description, u.answer, u.comment, u.is_last_answer
		FROM (SELECT * FROM {question_user_answers}
		  WHERE user_pw = :token) AS u
		LEFT JOIN {question} AS q ON q.question_id = u.question_id
		LEFT JOIN {question_possible_answers} AS a ON a.question_id = u.question_id AND a.weight = u.answer_id + 1 
    ORDER BY q.weight, u.answer_id, is_last_answer";

    $result = db_query($sql_query, array(':token' => $token));

    $result = $result->fetchAll();

    return $result;
  }

  /**
   * Returns the title of a thesis.
   *
   * @param integer $id
   *   The thesis ID.
   *
   * @return string
   *   The thesis title.
   *
   * @throws \Exception
   */
  public function delphi_evaluation_get_title($id) {

    $sql_query = "SELECT title FROM {question}
        WHERE question_id = :id";

    $result = db_query($sql_query, array(':id' => $id));

    $result = $result->fetchField();

    return $result;
  }


  /**
   * Returns the header for the CSV table.
   *
   * @param string $format
   *   Export format 'csv' or 'html'.
   *
   * @return array
   * @throws \Exception
   */
  public function delphi_evaluation_get_header($format) {

    $sql_query = "SELECT q.weight AS thesis_id, q.question_id, q.type AS type, q.title AS thesis_title, i.weight AS item_id, i.description AS item_title 
        FROM {question} AS q 
        LEFT JOIN question_possible_answers AS i 
        ON q.question_id = i.question_id 
        ORDER BY q.weight, i.weight ";

    $result = db_query($sql_query);

    $result = $result->fetchAll();
    $columns = array();
    $html = '';

    if ($format === 'html') {
      $groupId = 0;
      $tableArray = array();


      foreach ($result as $col) {
        if ($col->type === 'group') {
          $groupId = $col->thesis_id;
          $tableArray[$groupId]['group_title'] = $col->thesis_title;
        }
      }

      // build manual html table as Drupal's table theme does not support multirow headers
      $html = '<table><thead><tr>';
      foreach ($tableArray as $gId => $group) {

        $noCols = 0;


        foreach ($group as $tId => $thesis) {
          $noThesis = count($thesis) - 1;
          $titleThesis = $this->delphi_evaluation_get_title($gId);
          foreach ($thesis as $iId => $item) {
            $noCols = $noCols + 1;

          }
        }


        $noGroups = count($group) - 1;
        $titleGroup = $this->delphi_evaluation_get_title($gId);


        $html .= '<th colspan=' . $noCols . '>' . $titleGroup . '</th>';
      }

      $html .= '</tr></thead></tabble>';
    }

    $columns[] = 'Fall';
    foreach ($result as $col) {
      if ($col->type === 'group') {
        $columns[] = 'Einschätzung' . $col->thesis_id;
      } else {
        $prefix = 'These' . $col->thesis_id . '.Item' . $col->item_id;
        $columns[] = $prefix . '.init';
        $columns[] = $prefix . '.rev';
        $columns[] = $prefix . '.comm';
      }
    }

    if ($format === 'html') {
      return $html;
    } else {
      return $columns;
    }
  }

}