"""
SW_Services
2018-03-14
Used to subcalss and override certain services. 

WXCalculate in wxservices.py is overridden to add support for derived values in SW_Schema
StdWXCalculate is overriden to support the above

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
        weewx.engine.StdService.__init__(engine, config_dict)
        
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
    
    def __init__(self, config_dict, alt_vt, lat_f, long_f, db_binder=None):
        # Aditional quantities that this service knows how to calculate 
        _dispatch_list.list.extend([
            'barometerPRS',
            'barometerHouse',
            'dewpointFARS',
            'dewpointPRS'])
        
        super(SW_Calculate, self).__init__(config_dict, alt_vt, lat_f, long_f, db_binder)
        
        # get any configuration settings
        svc_dict = config_dict.get('StdWXCalculate', {'Calculations':{}})
        # if there is no Calculations section, then make an empty one
        if not 'Calculations' in svc_dict:
            svc_dict['Calculations'] = dict()
        
        # Super handles everything else. Hopefully.
        
    # do_calculations is very nicely extendable. No need to override. Hopefully.
    
    def calc_barometerPRS(self, data, data_type):  # @UnusedVariable
        data['barometerPRS'] = None
        if 'pressurePRS' in data and 'TempPRS' in data:
            data['barometerPRS'] = weewx.wxformulas.sealevel_pressure_US(
                data['pressurePRS'], self.altitude_ft, data['TempPRS'])
            
    def calc_barometerHouse(self, data, data_type):  # @UnusedVariable
        data['barometerHouse'] = None
        if 'pressureHouse' in data and 'TempHouse1' in data:
            data['barometerHouse'] = weewx.wxformulas.sealevel_pressure_US(
                data['pressureHouse'], self.altitude_ft, data['TempHouse1'])
            
    def calc_dewpointFARS(self, data, data_type):  # @UnusedVariable
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
            
    
        
        