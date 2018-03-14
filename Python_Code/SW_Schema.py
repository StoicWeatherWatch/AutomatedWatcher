
"""The stocic database schema
    18-03-14
    v1

This is a list containing the schema of the archive database. 
It is only used for initialization, afterwhich 
the schema is obtained dynamically from the database.  

"""

schema = [('dateTime',              'INTEGER NOT NULL UNIQUE PRIMARY KEY'),
          ('usUnits',               'INTEGER NOT NULL'),
          ('interval',              'INTEGER NOT NULL'),
          ('barometer',             'REAL'),
          ('pressurePRS',           'REAL'),
          ('pressureFARS',          'REAL'),
          ('pressureHouse',         'REAL'),
          ('TempHouse1',            'REAL'),
          ('TempHouse2',            'REAL'),
          ('TempFARS',              'REAL'),
          ('TempPRS',               'REAL'),
          ('HumidityHouse',         'REAL'),
          ('HumidityFARS',          'REAL'),
          ('HumidityPRS',           'REAL'),
          ('windSpeed',             'REAL'),
          ('windDir',               'REAL'),
          ('windGust',              'REAL'),
          ('windGustDir',           'REAL'),
          ('rainSmallTip',          'REAL'),
          ('rainLargeTip',          'REAL'),
          ('rainOptical',           'REAL'),
          ('IsRainingOptical',      'INTEGER'),
          ('dewpoint',              'REAL'),
          ('ET',                    'REAL'),
          ('UVindex',               'REAL'),
          ('OpticalRelativeLUX',    'REAL'),
          ('IRRelativeLUX',         'REAL'),
          ('extraTempBox',          'REAL'),
          ('extraTempFARS',         'REAL'),
          ('extraTempUV',           'REAL'),
          ('extraTemp1',            'REAL'),
          ('extraTemp2',            'REAL'),
          ('extraTemp3',            'REAL'),
          ('soilTemp1',             'REAL'),
          ('soilTemp2',             'REAL'),
          ('soilTemp3',             'REAL'),
          ('soilTemp4',             'REAL'),
          ('soilTemp5',             'REAL'),
          ('soilTempIR',            'REAL'),
          ('soilMoist1',            'REAL'),
          ('soilMoist2',            'REAL'),
          ('LightningStrikes',      'REAL'),
          ('LightningMeanDistance', 'REAL')]
