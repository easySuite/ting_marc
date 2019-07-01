/**
 * Fetch values for Ajaxified ting marcXchange fields.
 */
(function($) {
  var selector = 'ting-marc-unprocessed',
    processing = 'ting-marc-processing',
    nodata = 'ting-marc-nodata';

  function ting_marc_fields(container, settings) {
    var fields_obj = {};
    $('.' + selector, container).each(function(index, value) {
      var field = {
        data: $(this).data('ting-marc'),
        clickable: $(this).data('ting-clickable'),
        link_index: $(this).data('ting-link_index')
      };

      if (field.data) {
        fields_obj[index] = field;
      }
      $(this).removeClass(selector);
      $(this).addClass(processing);
    });

    if (!jQuery.isEmptyObject(fields_obj)) {
      $.ajax({
        url: Drupal.settings.basePath + Drupal.settings.pathPrefix + 'ting/marc/fields',
        dataType: 'json',
        type: 'POST',
        data: fields_obj,
        success: function (data) {
          $.each(data, function () {
            $('.' + processing, container).each(function (index) {
              var field = $(this),
                _data = $(field).data('ting-marc'),
                data_field = data[index];
              if (data_field) {
                if (typeof (data_field[_data]) === "undefined") {
                  field.addClass(nodata);
                }
                else {
                  field.find('.field-item').html(data_field[_data]);
                }
                field.removeClass(processing);
              }
            });
          });
        }
      });
    }
  }

  Drupal.behaviors.ting_marc = {
    attach : function(context, settings) {
      ting_marc_fields(context, settings);
    }
  };

})(jQuery);
