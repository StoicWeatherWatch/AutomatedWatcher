"""
SW_Services
2018-03-20
Used to subcalss and override certain services. 

WXCalculate in wxservices.py is overridden to add support for derived values in SW_Schema
StdWXCalculate is overriden to support the above

WXCalculate converts everything to US units. Bewear 

"""

# TODO things that the super class calculates have been left out of the new schema. Does this cause errors?

import syslog

import weedb
import weewx.units
import weewx.engine
import weewx.wxformulas
import weeutil.weeutil

from weewx.units import CtoF, mps_to_mph, kph_to_mph, METER_PER_FOOT

from weewx.wxservices import StdWXCalculate
from weewx.wxservices import WXCalculate

def logmsg(level, msg):
    syslog.syslog(level, 'SW_Services: %s' % msg)

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, 'ERROR: %s' % msg)


class SW_Std_Calculate(StdWXCalculate):
    """Wrapper class for SW_Calculate.

    A StdService wrapper for a SW_Calculate object so it may be called as a 
    service. This also allows the SW_Calculate class to be used elsewhere 
    without the overheads of running it as a weewx service.
    
    When upgrading WeeWX double check that the override below still works. Has super changed? We call weewx.engine.StdService explicitly.
    Does init need to do anything else?
    """
    
    def __init__(self, engine, config_dict):
        """Initialize the service.

        Create a SW_Calculate object and use super to initialise our bindings.
        """
        
        # This creates messy bindings. I have no idea if it would work. Never tried.
        #super(SW_Std_Calculate, self).__init__(engine, config_dict)
        
        # Explicit call to super super
        # This one does not seem to work
        #weewx.engine.StdService.__init__(engine, config_dict)
        # Another try
        super(StdWXCalculate, self).__init__(engine, config_dict)
        
        self.calc = SW_Calculate(config_dict, 
                                engine.stn_info.altitude_vt, 
                                engine.stn_info.latitude_f, 
                                engine.stn_info.longitude_f,
                                engine.db_binder)
        
        self.bind(weewx.NEW_LOOP_PACKET, self.new_loop_packet)
        self.bind(weewx.NEW_ARCHIVE_RECORD, self.new_archive_record)
    
    
class SW_Calculate(WXCalculate):
    """
    Add derived quantities to a record.
    
    Much of the work is done in super which is in wxservices.py aka weewx.wxservices.WXCalculate
    
    """
    
    # these are the quantities that this service knows how to calculate
    _dispatch_list = [
#        'windchill',
#        'heatindex',
#        'dewpoint',
#        'inDewpoint',
#        'maxSolarRad',
#        'cloudbase',
#        'humidex',
#        'appTemp',
#        'beaufort',
#        'ET',
#        'windrun',
        'barometerPRS',
        'barometerHouse',
        'dewpointFARS',
        'dewpointPRS',
        'outTemp',
        'inTemp',
        'outHumidity',
        'rain',
        'rainRate',
        'barometer']

    
    def __init__(self, config_dict, alt_vt, lat_f, long_f, db_binder=None):
        

        
        super(SW_Calculate, self).__init__(config_dict, alt_vt, lat_f, long_f, db_binder)
        
        # get any configuration settings
        self.SourceDest_dict = config_dict.get('SW_Std_Calculate', {'CopySources':{}})
        # if there is no CopySources section, then make an empty one
        if not 'CopySources' in self.SourceDest_dict:
            self.SourceDest_dict['CopySources'] = dict()
            
        
        
        # Super handles everything else. Hopefully.
        
    # do_calculations is very nicely extendable. No need to override. Hopefully.
    
    def calc_barometerPRS(self, data, data_type):  # @UnusedVariable
        data['barometerPRS'] = None
        if 'pressurePRS' in data and 'TempPRS' in data:
            if (data.get('pressurePRS') is not None) and (data.get('TempPRS') is not None):
                data['barometerPRS'] = weewx.wxformulas.sealevel_pressure_US(
                    data['pressurePRS'], self.altitude_ft, data['TempPRS'])
            
    def calc_barometerHouse(self, data, data_type):  # @UnusedVariable
        data['barometerHouse'] = None
        if 'pressureHouse' in data and 'TempHouse1' in data:
            if (data.get('pressureHouse') is not None) and (data.get('TempHouse1') is not None):
                data['barometerHouse'] = weewx.wxformulas.sealevel_pressure_US(
                    data['pressureHouse'], self.altitude_ft, data['TempHouse1'])
            
    def calc_dewpointFARS(self, data, data_type):
        #loginf(" calc_dewpointFARS Running")
        if 'TempFARS' in data and 'HumidityFARS' in data:
            data['dewpointFARS'] = weewx.wxformulas.dewpointF(
                data['TempFARS'], data['HumidityFARS'])
        else:
            data['dewpointFARS'] = None
            
    def calc_dewpointPRS(self, data, data_type):  # @UnusedVariable
        if 'TempPRS' in data and 'HumidityPRS' in data:
            data['dewpointPRS'] = weewx.wxformulas.dewpointF(
                data['TempPRS'], data['HumidityPRS'])
        else:
            data['dewpointPRS'] = None
            
    def calc_rainRate(self, data, data_type):  
        """
        We don't have a recorded vaiable called rain and this will run before it is calculated. Use whichever sourch is specified in 
        weewx.conf - SW_Std_Calculate - CopySources to give rain
        
        """
        #data['rain'] spits an exception on None get does not
        
        # first we check to see if we have any rain data...
        if self.SourceDest_dict['CopySources'].get('rainRate') in data:
            if data.get(self.SourceDest_dict['CopySources'].get('rainRate')) is not None:
        
                if (data.get('rain') == None) or ('rain' not in data):
                    if self.SourceDest_dict['CopySources'].get('rainRate') is not None:
                        self.calc_rain(data, data_type)
                else:
                    loginf("self.SourceDest_dict['CopySources'].get('rainRate') is None - ERROR")
                
            # Only try it if we have rain data
            super(SW_Calculate, self).calc_rainRate(data, data_type)
        
    def calc_barometer(self, data, data_type):
        # Set to None in case fo failuer
        data['barometer'] = None
        if (self.SourceDest_dict['CopySources'].get('barometer') in data) and (data.get(self.SourceDest_dict['CopySources'].get('barometer')) is not None):
            data['barometer'] = data[self.SourceDest_dict['CopySources'].get('barometer')]
        else:
            # it is assumed that the source can be caluclated
            getattr(self, 'calc_' + self.SourceDest_dict['CopySources'].get('barometer'))(data, data_type)
            # And try onece more 
            if (self.SourceDest_dict['CopySources'].get('barometer') in data) and (data.get(self.SourceDest_dict['CopySources'].get('barometer')) is not None):
                data['barometer'] = data[self.SourceDest_dict['CopySources'].get('barometer')]
                
    
    def calc_outTemp(self, data, data_type):
        if self.SourceDest_dict['CopySources'].get('outTemp') is not None:
            if self.SourceDest_dict['CopySources'].get('outTemp') in data:
                data['outTemp'] = data[self.SourceDest_dict['CopySources'].get('outTemp')]
                
        
    
    def calc_inTemp(self, data, data_type):
        if self.SourceDest_dict['CopySources'].get('inTemp') is not None:
            if self.SourceDest_dict['CopySources'].get('inTemp') in data:
                data['inTemp'] = data[self.SourceDest_dict['CopySources'].get('inTemp')]
                
    def calc_outHumidity(self, data, data_type):
        if self.SourceDest_dict['CopySources'].get('outHumidity') is not None:
            if self.SourceDest_dict['CopySources'].get('outHumidity') in data:
                data['outHumidity'] = data[self.SourceDest_dict['CopySources'].get('outHumidity')]
                

    def calc_rain(self, data, data_type):
        if data.get('rain') is not None:
            if self.SourceDest_dict['CopySources'].get('rain') is not None:
                if self.SourceDest_dict['CopySources'].get('rain') in data:
                    data['rain'] = data[self.SourceDest_dict['CopySources'].get('rain')]

        