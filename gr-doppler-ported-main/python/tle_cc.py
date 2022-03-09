#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2022 Conner Awald.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#


import numpy
from skyfield.api import *
import scipy.constants as consts
from gnuradio import gr
import datetime
import dateutil
from .cc import cc

class tle_cc(cc):
    """
    docstring for block tle_cc
    """
    def __init__(self, line1, line2, line3, lat, lon, hgt, utc_start, utc_end, 
                 N, center_freq, sample_rate, interpolate, edge_behavior, 
                 clock_type, negate):
        groundstation = wgs84.latlon(lat, lon) #Makes a geographicPosition object from skyfield
        groundstation.elevation = Distance(m = hgt) #Sets height to hgt meters
        sat = EarthSatellite(line2, line3)     #Line 1 is assumed to be the name or empty TLE line

        #Calculate various time objects here
        ts = Timescale()
        tstart = dateutil.parser.isoparse(utc_start)
        tend = dateutil.parser.isoparse(utc_end)
        tdelta = (tend - tstart).total_seconds()
        toffsets = range(0, tdelta)
        tlist = [tstart + datetime.timedelta(seconds=x) for x in toffsets]

        #Converts the list of datetime objects to skyfield objects
        skytimes = ts.from_datetimes(tlist)

        #Calculates a difference reference at each time t

        pos = (sat - groundstation).at(skytimes)

        #Gets the range rate (km/s) from the reference point of the groundstation
        #Returns as a list which each index corresponding to time t
        rangerate = pos.frame_latlon_and_rates(groundstation)[5] #Returns the range change rate

        #Converts to m/s
        rangerate = rangerate*1000

        #Shift is the offset that when added to the received freq, will place it at center freq
        #For spacecraft coming towards the receiver, shift will be a negative value
        shift = (rangerate/consts.c)*center_freq

        #Used for transmission doppler correction
        if negate:
            shift = [-x for x in shift]


        cc.__init__(self, sample_rate, interpolate, edge_behavior, toffsets, shift, clock_type)




        

