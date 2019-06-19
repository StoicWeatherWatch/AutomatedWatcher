
# StoicWS 2018-03-20

"""User extensions module

This module is imported from the main executable, so anything put here will be
executed before anything else happens. This makes it a good place to put user
extensions.
"""

# TODO make lightning count actually count up in archive records

# These two lines are the only two lines in WeeWX default extensions.
import locale
# This will use the locale specified by the environment variable 'LANG'
# Other options are possible. See:
# http://docs.python.org/2/library/locale.html#locale.setlocale
locale.setlocale(locale.LC_ALL, '')

# This is to get the measurments in the right group.

import weewx.units
weewx.units.obs_group_dict['barometerPRS'] = 'group_pressure'
weewx.units.obs_group_dict['barometerHouse'] = 'group_pressure'
weewx.units.obs_group_dict['barometerSpecial'] = 'group_pressure'
weewx.units.obs_group_dict['pressurePRS'] = 'group_pressure'
weewx.units.obs_group_dict['pressureFARS'] = 'group_pressure'
weewx.units.obs_group_dict['pressureHouse1'] = 'group_pressure'
weewx.units.obs_group_dict['pressureHouse2'] = 'group_pressure'
weewx.units.obs_group_dict['pressureSpecial'] = 'group_pressure'
weewx.units.obs_group_dict['pressureHydro'] = 'group_pressure'
weewx.units.obs_group_dict['TempHouse1'] = 'group_temperature'
weewx.units.obs_group_dict['TempHouse2'] = 'group_temperature'
weewx.units.obs_group_dict['TempFARS'] = 'group_temperature'
weewx.units.obs_group_dict['TempPRS'] = 'group_temperature'
weewx.units.obs_group_dict['HumidityHouse1'] = 'group_percent'
weewx.units.obs_group_dict['HumidityHouse2'] = 'group_percent'
weewx.units.obs_group_dict['HumidityFARS'] = 'group_percent'
weewx.units.obs_group_dict['HumidityPRS'] = 'group_percent'
weewx.units.obs_group_dict['HumidityHydro'] = 'group_percent'
weewx.units.obs_group_dict['HumidityExtra'] = 'group_percent'
# Wind measurments are set in WeeWX code
weewx.units.obs_group_dict['rainSmallTip'] = 'group_rain'
weewx.units.obs_group_dict['rainLargeTip'] = 'group_rain'
weewx.units.obs_group_dict['rainOptical'] = 'group_rain'
weewx.units.obs_group_dict['rainManual'] = 'group_rain'
weewx.units.obs_group_dict['IsRainingOptical'] = 'group_binary'
weewx.units.obs_group_dict['dewpointFARS'] = 'group_temperature'
weewx.units.obs_group_dict['dewpointPRS'] = 'group_temperature'
# ET measurments are set in WeeWX code
weewx.units.obs_group_dict['UVindex'] = 'group_uv'
weewx.units.obs_group_dict['OpticalRelativeLUX'] = 'group_relative_light'
weewx.units.obs_group_dict['IRRelativeLUX'] = 'group_relative_light'
weewx.units.obs_group_dict['extraTempBox'] = 'group_temperature'
weewx.units.obs_group_dict['extraTempFARS'] = 'group_temperature'
weewx.units.obs_group_dict['extraTempUV'] = 'group_temperature'
weewx.units.obs_group_dict['TempHydro'] = 'group_temperature'
weewx.units.obs_group_dict['extraTempCPU'] = 'group_temperature'
# Extra temp 1-4 are set in WeeWX code
# Soil temp 1-4 are set in WeeWX code
weewx.units.obs_group_dict['soilTemp5'] = 'group_temperature'
weewx.units.obs_group_dict['soilTempIR'] = 'group_temperature'
# soilMoist 1-2 are set in WeeWX code
weewx.units.obs_group_dict['LightningStrikes'] = 'group_count'
weewx.units.obs_group_dict['LightningDistance'] = 'group_distance'
# rainRate, barometer, outTemp, inTemp, outHumidity, rain are set in WeeWX code
# Extra data 1-2 should be set here to be used


# New unit group group_binary
# a yes or no state. 
# isRaining yes 1 no 0

weewx.units.USUnits['group_binary'] = 'binary_state'
weewx.units.MetricUnits['group_binary'] = 'binary_state'
weewx.units.MetricWXUnits['group_binary'] = 'binary_state'

weewx.units.default_unit_format_dict['binary_state'] = '%d'

weewx.units.default_unit_label_dict['binary_state'] = ' (1 for YES 0 for NO)'


# New unit group relative light intensity. A measure of light but uncalibrated
weewx.units.USUnits['group_relative_light'] = 'relative_light'
weewx.units.MetricUnits['group_relative_light'] = 'relative_light'
weewx.units.MetricWXUnits['group_relative_light'] = 'relative_light'

weewx.units.default_unit_format_dict['relative_light'] = '%.1f'

weewx.units.default_unit_label_dict['relative_light'] = ' Relative Light Intensity'


# New unit group to count events. Lightning strikes or at least intruppts on the lightning detector
# weewx.units.USUnits['group_count'] = 'count'
# weewx.units.MetricUnits['group_count'] = 'count'
# weewx.units.MetricWXUnits['group_count'] = 'count'
# 
# weewx.units.default_unit_format_dict['count'] = '%d'
# 
# weewx.units.default_unit_label_dict['count'] = ' '

