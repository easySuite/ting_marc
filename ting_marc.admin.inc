<?php
/**
 * @file
 * ting_marc admin functions.
 */

define('STATUS_INSERT', 1);
define('STATUS_UPDATE', 2);

/**
 * Show current settings.
 *
 * @return string
 *   HTML.
 */
function ting_marc_overview_page() {
  $result = db_query("SELECT * FROM {ting_marc} ORDER BY ting_type, marc_field, marc_subfield, field_clickable, link_index", array(), array('fetch' => PDO::FETCH_ASSOC));

  $types = ting_marc_types();

  $header = array(
    t('Type'),
    t('Field'),
    t('Subfield'),
    t('Label'),
    t('Field name'),
    t('Clickable'),
    t('Link index'),
    array('data' => t('Operations'), 'colspan' => '2'),
  );
  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row[] = array('data' => $types[$data['ting_type']]);
    $row[] = array('data' => $data['marc_field']);
    $row[] = array('data' => $data['marc_subfield']);
    $row[] = array('data' => $data['marc_label']);
    $row[] = array('data' => $data['field_name']);
    $row[] = array('data' => $data['field_clickable']);
    $row[] = array('data' => $data['link_index']);
    $row[] = array('data' => l(t('edit'), TING_MARC_ADMIN . '/' . $data['id'] . '/edit'));
    $row[] = array('data' => l(t('delete'), TING_MARC_ADMIN . '/' . $data['id'] . '/delete'));
    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Settings form.
 */
function ting_marc_edit_item($form, &$form_state, $action, $item = NULL) {
  $form['action'] = array(
    '#type' => 'hidden',
    '#default_value' => $action,
  );

  $form['id'] = array(
    '#type' => 'hidden',
  );
  if ($action == 'add' || empty($item)) {
    $item = array(
      'ting_type' => '',
      'marc_field' => '',
      'marc_subfield' => '',
      'marc_label' => '',
      'field_name' => '',
      'field_clickable' => '',
      'link_index' => '',
    );
  }
  else {
    $res = db_query("SELECT * FROM {ting_marc} WHERE id=:id",
      array('id' => $item),
      array('fetch' => PDO::FETCH_ASSOC)
    );

    $item = $res->fetchAssoc();

    $form['id']['#default_value'] = $item['id'];
  }

  $types = ting_marc_types();

  $form['ting_type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => $types,
    '#default_value' => $item['ting_type'],
    '#description' => t('Ting item type'),
  );

  $form['marc_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Marc field'),
    '#default_value' => $item['marc_field'],
    '#required' => TRUE,
  );

  $form['marc_subfield'] = array(
    '#type' => 'textfield',
    '#title' => t('Marc subfield'),
    '#default_value' => $item['marc_subfield'],
    '#required' => TRUE,
  );

  $form['marc_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => $item['marc_label'],
    '#required' => TRUE,
  );
  $form['clickable'] = array(
    '#type' => 'checkbox',
    '#title' => t('This field should be clickable'),
    '#description' => t('If selected, values from this MARC field will be clickable. </br> All values will be separated by comma and clickable.'),
    '#default_value' => $item['field_clickable'],
  );
  $link_address = l(
    'http://opensearch.addi.dk/4.0.1/opensearch_cql.xml',
    'http://opensearch.addi.dk/4.0.1/opensearch_cql.xml',
    array('attributes' => array('target' => '_blank'))
  );
  $form['link_index'] = array(
    '#type' => 'textfield',
    '#title' => t('Enter the index to link to:'),
    '#attributes' => array('placeholder' => array(t('For example: phrase.creator'))),
    '#default_value' => $item['link_index'],
    '#description' => t('Link indexes can be found at !link', array('!link' => $link_address)),
  );
  $form['field_name'] = array(
    '#type' => 'hidden',
    '#default_value' => $item['field_name'],
  );

  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}

/**
 * Save setting.
 */
function ting_marc_edit_item_submit($form, &$form_state) {
  if (!function_exists('transliteration_get')) {
    module_load_include('module', 'transliteration');
  }

  $item = $form_state['values'];
  if (empty($item['field_name'])) {
    $value = drupal_html_class($item['ting_type'] . '_' . $item['marc_label']);
    $value = str_replace('-', '_', $value);
    $value = transliteration_get($value);
    $item['field_name'] = $value;
  }
  $fields = array(
    'ting_type' => $item['ting_type'],
    'marc_field' => $item['marc_field'],
    'marc_subfield' => $item['marc_subfield'],
    'marc_label' => $item['marc_label'],
    'field_name' => $item['field_name'],
    'field_clickable' => $item['clickable'],
    'link_index' => $item['link_index'],
  );

  $result = db_merge('ting_marc')
    ->key(array('field_name' => $item['field_name']))
    ->fields($fields)
    ->execute();

  if ($result == STATUS_UPDATE) {
    drupal_set_message(t('Field @name was successfully updated. If you want to create another field insert another type or label.', array('@name' => $item['field_name'])));
  }

  ting_marc_create_field($fields);

  $form_state['redirect'] = TING_MARC_ADMIN;
}

/**
 * Delete setting page.
 */
function ting_marc_delete_item($id) {
  return drupal_get_form('ting_marc_delete_item_confirm', $id);
}

/**
 * Delete confirmation form/page.
 */
function ting_marc_delete_item_confirm($form, &$form_state, $id) {
  $form['#marc_id'] = $id;
  $caption = '<p>' . t('This action cannot be undone.') . '</p>';
  return confirm_form(
    $form,
    t('Are you sure you want to delete this setting?'),
    TING_MARC_ADMIN,
    filter_xss($caption),
    t('Delete')
  );
}

/**
 * Delete setting.
 */
function ting_marc_delete_item_confirm_submit($form, &$form_state) {
  $id = $form['#marc_id'];
  $form_state['redirect'] = TING_MARC_ADMIN;

  $field = db_select('ting_marc', 'tm')
    ->fields('tm', array('field_name'))
    ->condition('id', $id, '=')
    ->execute()
    ->fetchAssoc();

  db_delete('ting_marc')
    ->condition('id', $id)
    ->execute();

  _ting_marc_purge_field($field['field_name']);
  drupal_set_message(t('Field has been deleted.'));
}

/**
 * Ting types.
 *
 * @param bool $pair
 *   Return pairs as key => value or just values.
 *
 * @return array
 *   Ting types.
 */
function ting_marc_types() {
  $ting_well_types = variable_get('ting_well_types', array());

  // If we don't have any well types they might not have been fetched yet.
  // TODO: Maybe it should be possible to update the well types on the form
  // instead. Like it's done in ting module.
  if (empty($ting_well_types)) {
    $ting_well_types = _ting_fetch_well_types();
  }

  $type_arr = drupal_map_assoc(array_keys($ting_well_types));

  array_walk($type_arr, function (&$item) {
    $item = t(ucfirst($item));
  });
  asort($type_arr);
  $result = array('all' => t('All')) + $type_arr;

  return $result;
}


/**
 * Form builder for module settings form.
 */
function ting_marc_module_settings($form, $form_state) {
  $form = array();

  $form['ting_mark_logging'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable logging'),
    '#default_value' => variable_get('ting_mark_logging', 0),
    '#description' => t("Log all errors to watchdog."),
  );

  return system_settings_form($form);
}
