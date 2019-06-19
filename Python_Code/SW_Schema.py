
"""The stocic database schema
    2019-6-16
    v2
    
    user.SW_Schema.schema

This is a list containing the schema of the archive database. 
It is only used for initialization, afterwhich 
the schema is obtained dynamically from the database.  

barometer is pressure adjusted to sea level pressure and for tempreture. 


"""

# TODO add quality checks to weewx.conf for new names

schema = [('dateTime',              'INTEGER NOT NULL UNIQUE PRIMARY KEY'),
          ('usUnits',               'INTEGER NOT NULL'),
          ('interval',              'INTEGER NOT NULL'),
          ('barometerPRS',          'REAL'),
          ('barometerHouse',        'REAL'),
          ('barometerSpecial',      'REAL'),
          ('pressurePRS',           'REAL'),
          ('pressureFARS',          'REAL'),
          ('pressureHouse1',        'REAL'),
          ('pressureHouse2',        'REAL'),
          ('pressureSpecial',       'REAL'),
          ('pressureHydro',         'REAL'),
          ('TempHouse1',            'REAL'),
          ('TempHouse2',            'REAL'),
          ('TempFARS',              'REAL'),
          ('TempPRS',               'REAL'),
          ('HumidityHouse1',        'REAL'),
          ('HumidityHouse2',        'REAL'),
          ('HumidityFARS',          'REAL'),
          ('HumidityPRS',           'REAL'),
          ('HumidityHydro',         'REAL'),
          ('HumidityExtra',         'REAL'),
          ('windSpeed',             'REAL'),
          ('windDir',               'REAL'),
          ('windGust',              'REAL'),
          ('windGustDir',           'REAL'),
          ('rainSmallTip',          'REAL'),
          ('rainLargeTip',          'REAL'),
          ('rainOptical',           'REAL'),
          ('rainManual',            'REAL'),
          ('IsRainingOptical',      'INTEGER'),
          ('dewpointFARS',          'REAL'),
          ('dewpointPRS',           'REAL'),
          ('ET',                    'REAL'),
          ('UVindex',               'REAL'),
          ('OpticalRelativeLUX',    'REAL'),
          ('IRRelativeLUX',         'REAL'),
          ('radiation',             'REAL'),
          ('extraTempBox',          'REAL'),
          ('extraTempFARS',         'REAL'),
          ('extraTempUV',           'REAL'),
          ('TempHydro',             'REAL'),
          ('extraTempCPU',          'REAL'),
          ('extraTemp1',            'REAL'),
          ('extraTemp2',            'REAL'),
          ('extraTemp3',            'REAL'),
          ('extraTemp4',            'REAL'),
          ('soilTemp1',             'REAL'),
          ('soilTemp2',             'REAL'),
          ('soilTemp3',             'REAL'),
          ('soilTemp4',             'REAL'),
          ('soilTemp5',             'REAL'),
          ('soilTempIR',            'REAL'),
          ('soilMoist1',            'REAL'),
          ('soilMoist2',            'REAL'),
          ('LightningStrikes',      'INTEGER'),
          ('LightningDistance',     'REAL'),
          ('ExtraData1',            'REAL'),
          ('ExtraData2',            'REAL'),
          ('rainRate',                     'REAL'),
          ('barometer',                    'REAL'),
          ('outTemp',                      'REAL'),
          ('inTemp',                       'REAL'),
          ('outHumidity',                  'REAL'),
          ('rain',                         'REAL')]

# To use ExtraData add it to a group under extensions

# Cheetahgenerator usess 'barometer' and 'outTemp' 
# To avoid issues 'rainRate', 'barometer','outTemp','outHumidity','rain', 'inTemp' are retained. SW_Services will calculate them
