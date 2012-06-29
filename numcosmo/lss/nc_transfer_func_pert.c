/***************************************************************************
 *            nc_transfer_func_pert.c
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
 * SECTION:nc_transfer_func_pert
 * @title: Pert Transfer Function
 * @short_description: FIXME
 *
 * FIXME
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */
#include <numcosmo/numcosmo.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>

G_DEFINE_TYPE (NcTransferFuncPert, nc_transfer_func_pert, NC_TYPE_TRANSFER_FUNC);

/**
 * nc_transfer_func_pert_new:
 *   
 * FIXME
 *
 * Returns: A new #NcTransferFunc.
 */
NcTransferFunc *
nc_transfer_func_pert_new ()
{
  return g_object_new (NC_TYPE_TRANSFER_FUNC_PERT, NULL);
}


static void
_nc_transfer_func_pert_prepare (NcTransferFunc *tf, NcHICosmo *model)
{
  NcTransferFuncPert *tf_pert = NC_TRANSFER_FUNC_PERT (tf);
  if (!tf_pert->init)
  {
	tf_pert->pert = nc_pert_linear_new (NCM_MODEL (model), 1 << 3, 1e-7, 1e-7, 1e-10, 1e-10);
	tf_pert->pspline = nc_pert_linear_splines_new (tf_pert->pert, NC_LINEAR_PERTURBATIONS_SPLINE_PHI, 60, 220, 1.0e-2, NC_C_HUBBLE_RADIUS * 1000.0);
	tf_pert->init = TRUE;
  }
  nc_pert_linear_prepare_splines (tf_pert->pspline);
}

static gdouble
_nc_transfer_func_pert_calc (NcTransferFunc *tf, gdouble kh)
{
  NcTransferFuncPert *tf_pert = NC_TRANSFER_FUNC_PERT (tf);
  return ncm_spline_eval (NC_LINEAR_PERTURBATIONS_GET_SPLINE (tf_pert->pspline, NC_PERT_PHI), kh * NC_C_HUBBLE_RADIUS);
}

static gdouble
_nc_transfer_func_pert_calc_matter_P (NcTransferFunc *tf, NcHICosmo *model, gdouble kh)
{
  gdouble T = _nc_transfer_func_pert_calc (tf, kh);
  return T * T * nc_hicosmo_powspec (model, kh);
}

static void
nc_transfer_func_pert_init (NcTransferFuncPert *tf_pert)
{
  /* TODO: Add initialization code here */
  tf_pert->pert = NULL;
  tf_pert->init = FALSE;
  tf_pert->pspline = NULL;
}

static void
_nc_transfer_func_pert_dispose (GObject *object)
{
  /* TODO: Add deinitalization code here */
  NcTransferFuncPert *tf_pert = NC_TRANSFER_FUNC_PERT (object);
  if (tf_pert->pert != NULL)
	nc_pert_linear_free (tf_pert->pert);
  if (tf_pert->pspline != NULL)
	nc_pert_linear_splines_free (tf_pert->pspline);
	
  G_OBJECT_CLASS (nc_transfer_func_pert_parent_class)->finalize (object);
}

static void
_nc_transfer_func_pert_finalize (GObject *object)
{
  /* TODO: Add deinitalization code here */

  G_OBJECT_CLASS (nc_transfer_func_pert_parent_class)->finalize (object);
}

static void
nc_transfer_func_pert_class_init (NcTransferFuncPertClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  NcTransferFuncClass* parent_class = NC_TRANSFER_FUNC_CLASS (klass);

  parent_class->prepare = &_nc_transfer_func_pert_prepare;
  parent_class->calc = &_nc_transfer_func_pert_calc;
  parent_class->calc_matter_P = &_nc_transfer_func_pert_calc_matter_P;

  object_class->dispose = _nc_transfer_func_pert_dispose;
  object_class->finalize = _nc_transfer_func_pert_finalize;
}
