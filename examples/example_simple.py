#!/usr/bin/python2

import math
import gi
gi.require_version('NumCosmo', '1.0')
gi.require_version('NumCosmoMath', '1.0')

from gi.repository import NumCosmo as Nc
from gi.repository import NumCosmoMath as Ncm

#
#  Initializing the library objects, this must be called before 
#  any other library function.
#
Ncm.cfg_init ()

#
#  New homogeneous and isotropic cosmological model NcHICosmoDEXcdm 
#
cosmo = Nc.HICosmo.new_from_name (Nc.HICosmo, "NcHICosmoDEXcdm")
cosmo.set_reparam (Nc.HICosmoDEReparamCMB.new (cosmo.len ()))

#
#  New cosmological distance objects optimizied to perform calculations
#  up to redshift 2.0.
#
dist = Nc.Distance.new (2.0)

#
#  Setting values for the cosmological model, those not set stay in the
#  default values. Remember to use the _orig_ version to set the original
#  parameters when a reparametrization is used.
#

#
# C-like
#
cosmo.orig_param_set (Nc.HICosmoDEParams.H0,       70.00)
cosmo.orig_param_set (Nc.HICosmoDEParams.OMEGA_C,   0.25)
cosmo.orig_param_set (Nc.HICosmoDEParams.OMEGA_X,   0.70)
cosmo.orig_param_set (Nc.HICosmoDEParams.T_GAMMA0,  2.72)
cosmo.orig_param_set (Nc.HICosmoDEParams.OMEGA_B,   0.05)
cosmo.orig_param_set (Nc.HICosmoDEParams.SPECINDEX, 1.00)
cosmo.orig_param_set (Nc.HICosmoDEParams.SIGMA8,    0.90)
cosmo.orig_param_set (Nc.HICosmoDEXCDMParams.W,    -1.10)

#
# OO-like
#
cosmo.props.H0      = 70.00
cosmo.props.Omegab  =  0.04
cosmo.props.Omegac  =  0.25
cosmo.props.Omegax  =  0.70
cosmo.props.Tgamma0 =  2.72
cosmo.props.ns      =  1.00
cosmo.props.sigma8  =  0.90
cosmo.props.w       = -1.10

#
#  Printing the parameters used.
#
print "# Model parameters: ", 
cosmo.params_log_all ()

#
#  Printing some distances up to redshift 1.0.
#
for i in range (0, 10):
  z = 1.0 / 9.0 * i
  cd = Ncm.C.hubble_radius () * dist.comoving (cosmo, z)
  print "% 10.8f % 20.15g" % (z, cd)


dist = Nc.Distance.new (2000.0)

dist.prepare (cosmo)

print dist.theta100CMB (cosmo)