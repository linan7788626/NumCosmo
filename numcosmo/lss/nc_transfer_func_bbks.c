/***************************************************************************
 *            nc_transfer_func_bbks.c
 *
 *  Mon Jun 28 15:09:13 2010
 *  Copyright  2010  Mariana Penna Lima
 *  <pennalima@gmail.com>
 ****************************************************************************/
/*
 * numcosmo
 * Copyright (C) Mariana Penna Lima 2012 <pennalima@gmail.com>
 *
 * numcosmo is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * numcosmo is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:nc_transfer_func_bbks
 * @title: NcTransferFuncBBKS
 * @short_description: Bardeen, Bond, Kaiser and Szalay (BBKS) transfer function.
 * 
 * FIXME
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */
#include "build_cfg.h"

#include "lss/nc_transfer_func_bbks.h"

G_DEFINE_TYPE (NcTransferFuncBBKS, nc_transfer_func_bbks, NC_TYPE_TRANSFER_FUNC);

static void
nc_transfer_func_bbks_init (NcTransferFuncBBKS *tf_bbks)
{
  tf_bbks->c1    = 0.0;
  tf_bbks->c2    = 0.0;
  tf_bbks->c3    = 0.0;
  tf_bbks->c4    = 0.0;
  tf_bbks->c5_wm = 0.0;
  tf_bbks->h     = 0.0;
}

static void
nc_transfer_func_bbks_finalize (GObject *object)
{

  /* Chain up : end */
  G_OBJECT_CLASS (nc_transfer_func_bbks_parent_class)->finalize (object);
}

static void _nc_transfer_func_bbks_prepare (NcTransferFunc *tf, NcHICosmo *cosmo);
static gdouble _nc_transfer_func_bbks_calc (NcTransferFunc *tf, gdouble kh);

static void
nc_transfer_func_bbks_class_init (NcTransferFuncBBKSClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  NcTransferFuncClass* parent_class = NC_TRANSFER_FUNC_CLASS (klass);

  parent_class->prepare = &_nc_transfer_func_bbks_prepare;
  parent_class->calc = &_nc_transfer_func_bbks_calc;

  object_class->finalize = nc_transfer_func_bbks_finalize;
}

/**
 * nc_transfer_func_bbks_new:
 *
 * FIXME
 *
 * Returns: A new #NcTransferFunc.
 */
NcTransferFunc *
nc_transfer_func_bbks_new ()
{
  return g_object_new (NC_TYPE_TRANSFER_FUNC_BBKS, NULL);
}

static void
_nc_transfer_func_bbks_prepare (NcTransferFunc *tf, NcHICosmo *cosmo)
{
  NcTransferFuncBBKS *tf_BBKS = NC_TRANSFER_FUNC_BBKS (tf);
  const gdouble T_0 = nc_hicosmo_T_gamma0 (cosmo);
  const gdouble c1  = 3.89;
  const gdouble c2  = gsl_pow_2 (16.1);
  const gdouble c3  = gsl_pow_3 (5.46);
  const gdouble c4  = gsl_pow_4 (6.71);
  const gdouble c5  = gsl_pow_2 (T_0 / 2.7);   /* CMB: (T_0/2.7)^2 = (2.725/2.7)^2 */
  const gdouble h   = nc_hicosmo_h (cosmo);
  const gdouble h2  = h * h;
  const gdouble wm  = nc_hicosmo_Omega_m0 (cosmo) * h2;

  tf_BBKS->c1    = c1;
  tf_BBKS->c2    = c2;
  tf_BBKS->c3    = c3;
  tf_BBKS->c4    = c4;
  tf_BBKS->h     = h;
  tf_BBKS->c5_wm = c5 / wm;
}

static gdouble
_nc_transfer_func_bbks_calc (NcTransferFunc *tf, gdouble kh)
{
  NcTransferFuncBBKS *tf_BBKS = NC_TRANSFER_FUNC_BBKS (tf);
  const gdouble k = kh * tf_BBKS->h;
  const gdouble q = k * tf_BBKS->c5_wm;
  const gdouble q1 = 2.34 * q;
  const gdouble q2 = q * q;
  const gdouble q3 = q2 * q;
  const gdouble q4 = q3 * q;

  return (q1 == 0.0 ? 1.0 : (log1p (q1) / q1)) * pow (1.0 + tf_BBKS->c1 * q + tf_BBKS->c2 * q2 + tf_BBKS->c3 * q3 + tf_BBKS->c4 * q4, -1.0 / 4.0);
}
