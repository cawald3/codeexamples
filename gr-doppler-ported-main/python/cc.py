#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2022 Conner Awald.
#
# Steve Hall authored most of this code back in 2018 and this code
# was ported over to gnuradio 3.8 by Conner Awald in 2022 with minor
# modifications
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



from datetime import datetime
from gnuradio import gr
import numpy as np
from scipy.interpolate import interp1d 

class interp:
    """
    Helper class for interpolation models. Require: scipy.interpolate

    For interpretation, the x axis of the data MUST start at 0, all calls
    to this class assume that the interpolation was made with 0 representing
    the first sample, start time, etc...

    
    """
    valid_interpolation = [None, "cubic", "linear", "nearest"]
    valid_edge = ["maintain", "extrapolate", "zero"]

    def __init__(self, interp_type="cubic", edge_behavior="maintain") -> None: 
        if interp_type not in interp.valid_interpolation:
            raise ValueError("invalid interpolation \"%s\" given to interp" %interp_type)
        self.d_interp_type = interp_type
        self.d_edge_behavior = edge_behavior
        self.d_model = None
        self.x = None
        self.y = None

    @property
    def edge_behavior(self):
        return self.d_edge_behavior

    @edge_behavior.setter
    def edge_behavior(self, value):
        if value not in self.valid_edge:
            raise ValueError("Invalid edge behavior given to interp")
        self.d_edge_behavior = value
        if self.d_model is not None:
            if self.d_edge_behavior == "maintain":
                self.d_model.fill_value = self.y[-1]
            elif self.d_edge_behavior == "extrapolate":
                self.d_model.fill_value = "extrapolate"
            else:
                self.d_model.fill_value = 0
    
    @property
    def interp_type(self):
        return self.d_interp_type
    
    @interp_type.setter
    def interp_type(self, value):
        if value not in self.valid_interpolation:
            raise ValueError("Invalid interpolation type given to interp")
        self.d_interp_type = value
        if self.x is not None and self.y is not None:
            self.model(self.x,self.y)

    

    def model(self, x, y):
        """
        Ingests a point set and computes a model for them based on interp_type
        """
        self.x = x
        self.y = y
        #The following code is a refactor from Steve Halls' Code, I'm not sure why he had it setup
        #to force a call of the edge_behavior setter when this interp1d call should work fine
        if self.d_interp_type is not None:
            self.d_model = interp1d(x, y, kind=self.d_interp_type,
                                    bounds_error=False, fill_value=self.d_model.fill_value)

    def predict(self, x):
        """
        Uses the interpolation model to calculate the value at x. x can be a tuple
        Returns 0.0 if a valid model is not available
        """
        if isinstance(self.d_model, interp1d):
            return self.d_model(x)
        else:
            return 0.0
    
class ClockType(Enum):
    SAMPLE = 1
    SYSTEM = 2



class cc(gr.sync_block):
    """
    Applies doppler shift to incoming samples based on an interpolation model 
    For every sample in, this block will produce one sample out
    \nTime is given in seconds,
    \nShift is supplied in Hz
    """
    def __init__(self, sample_rate, interpolate, edge_behavior, time, shift, clock_type) -> None:

        gr.sync_block.__init__(self,
            name="cc",
            in_sig=[numpy.csingle, ],       #Specifies the input signals to be of type numpy single precision float complex
            out_sig=[numpy.csingle, ])     #Specifies the output signals to be of type numpy single precision float complex

        #If using pylance, the following lines will show up as unreachable due to the missing return hint
        #for the previous gr method, this is probably due to backwards compatibility with python2
        self.d_init = time is not None and shift is not None and time and shift

        self.interpolator = interp(interpolate, edge_behavior)

        if self.d_init:
            if len(time) != len(shift):
                raise ValueError("Time and Shift must be equal length")


        #This line initializes the model for the shifts at various times
        self.interpolator.model(time, shift)
        
        #Stored sample rate in samples per second
        self.sample_rate = float(sample_rate)

        #Precompute hz to rad multiplier
        self.hz_to_rad = 2.0 * np.pi / self.sample_rate 

        #Sample index/count
        self.count = 0

        #Phase tracker
        self.phase = 0

        # start_time is used when clock type is system_clock
        self.start_time = datetime.utcnow()

        if clock_type == "sample_based":
            self.clock_type = ClockType.SAMPLE
        elif clock_type == "system_clock":
            self.clock_type = ClockType.SYSTEM
        else:
            raise ValueError(
                "Clock type must be \"sample_based\" or \"system_clock\"")

    def shift(self, x):
        n = len(x)

        #Calculate time, frequency, and complex corrector for each sample
        #This sample clocktype also deviates from Steve's code, I'm not quite
        #sure why he was using np.array to do this
        if self.clock_type == ClockType.SAMPLE:
            counts = np.arange(0,n) + self.count
            times = counts / self.sample_rate
        else:
            now = datetime.utcnow()
            offset = (now - self.start_time).total_seconds()

        #TO-DO Implement the rest of the shifting
        
        interpshift = self.interpolator.predict(offset)




    def work(self, input_items, output_items):
        in0 = input_items[0]
        out = output_items[0]
        # <+signal processing here+>
        out[:] = in0
        return len(output_items[0])

