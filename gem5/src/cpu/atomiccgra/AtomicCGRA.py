# Copyright (c) 2012 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2007 The Regents of The University of Michigan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Nathan Binkert

from m5.params import *
from BaseCGRA import BaseCGRA

class AtomicCGRA(BaseCGRA):
    """Simple CPU model executing a configurable number of
    instructions per cycle. This model uses the simplified 'atomic'
    memory mode."""

    type = 'AtomicCGRA'
    cxx_header = "cpu/atomiccgra/atomiccgra.hh"

    @classmethod
    def memory_mode(cls):
        return 'atomic'

    @classmethod
    def support_take_over(cls):
        return True

    dcache_port1 = MasterPort("Data Port1")
    dcache_port2 = MasterPort("Data Port2")
    dcache_port3 = MasterPort("Data Port3")
    dcache_port4 = MasterPort("Data Port4")

    _data_ports = ['dcache_port1', 'dcache_port2', 'dcache_port3','dcache_port4']

    def connectDataPorts(self, bus, uncached_bus = None):
        for p in self._data_ports:
            exec('self.%s = bus.slave' % p)

    width = Param.Int(1, "CPU width")
    simulate_data_stalls = Param.Bool(False, "Simulate dcache stall cycles")
    simulate_inst_stalls = Param.Bool(False, "Simulate icache stall cycles")
    fastmem = Param.Bool(False, "Access memory directly")
    simpoint_profile = Param.Bool(False, "Generate SimPoint BBVs")
    simpoint_interval = Param.UInt64(100000000, "SimPoint Interval Size (insts)")
    simpoint_profile_file = Param.String("simpoint.bb.gz", "SimPoint BBV file")
