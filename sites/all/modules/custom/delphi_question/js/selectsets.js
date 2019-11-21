(function ($, Drupal) {

  Drupal.behaviors.delphi_question = {
    attach: function (context, settings) {

      // update values according to selected answer set
      $('.answer-sets').change(function() {

        // get associated answer set of user selection
        var answerSet = Drupal.settings.delphi_question.sets[this.selectedIndex-1];
        var setCount = answerSet.length;
        var fieldsetId = this.id.replace('edit-answer-sets','');

        // update radio buttons for the number of levels of the Likert scale
        $('#edit-button-radios' + fieldsetId + '-' + setCount).click().trigger('change');

        // update Likert scale levels
        $('[name^=textfield][id$=button' + fieldsetId + ']').each(function(index) {
          this.value = answerSet[index];
        });
      });
    }
  };
})(jQuery, Drupal);
