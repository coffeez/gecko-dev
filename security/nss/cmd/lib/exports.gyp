# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
{
  'includes': [
    '../../coreconf/config.gypi'
  ],
  'targets': [
    {
      'target_name': 'cmd_lib_exports',
      'type': 'none',
      'copies': [
        {
          'files': [
            'basicutil.h',
            'pk11table.h',
            'secutil.h'
          ],
          'destination': '<(PRODUCT_DIR)/dist/<(module)/private'
        }
      ]
    }
  ],
  'variables': {
    'module': 'nss'
  }
}