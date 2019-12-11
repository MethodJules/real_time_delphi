(function ($, Drupal) {

    Drupal.behaviors.survey_create_boxplot = {
      attach: function (context, settings) {


        $('.schild').hover(
            console.log('Hover over schild')
        );
      }
    };
  })(jQuery, Drupal);
