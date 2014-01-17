/**
 * Fetch values for Ajaxified ting marcXchange fields.
 */
(function($) {
  var hidden = 'item-collapsed',
    selector = 'ting-marc-unprocessed',
    processing = 'ting-marc-processing',
    nodata = 'ting-marc-nodata',
    visible_container = 'pane-entity-field';

  function ting_marc_fields(container) {
    var fields = [];

    $('.' + selector, container).each(function() {
      var data = $(this).data('ting-marc');
      if (data) {
        fields.push(data);
      }
      $(this).removeClass(selector);
      $(this).addClass(processing);
    });

    if (fields.length > 0) {
      $.post(
        Drupal.settings.basePath + Drupal.settings.pathPrefix + 'ting/marc/fields',
        {ting_marc_fields: fields},
        function(data) {
          $('.' + processing, container).each(function(){
            var field = $(this),
              _data = field.data('ting-marc');

            if (typeof(data[_data]) == "undefined") {
              field.addClass(nodata);
            }
            else {
              field.find('.field-item').html(data[_data]);
            }
            field.removeClass(processing);
          });
        }
      );
    }
  }

  function show_for_opened(container) {
    $('.' + visible_container, container).each(function(){
      if (!$(this).hasClass('hidden')) {
        ting_marc_fields(this);
      }
    });
  }

  Drupal.behaviors.ting_marc = {
    attach : function(context, settings) {
      // Fetch data for visible fields.
      show_for_opened(context);

      // Fetch data when "details" is opened.
      $('legend', context).click(function() {
        var parent = $(this).parent().parent();
        if (!parent.hasClass(hidden)) {
          ting_marc_fields(parent);
        }
      });
      $('.show-info', context).click(function() {
        show_for_opened(context);
      });
    }
  };

})(jQuery);
