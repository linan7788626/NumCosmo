/***************************************************************************
 *            nc_hicosmo_qspline.c
 *
 *  Wed February 15 11:31:28 2012
 *  Copyright  2012  Sandro Dias Pinto Vitenti
 *  <sandro@isoftware.com.br>
 ****************************************************************************/
/*
 * numcosmo
 * Copyright (C) Sandro Dias Pinto Vitenti 2012 <sandro@isoftware.com.br>
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
 * SECTION:nc_hicosmo_qspline
 * @title: Spline Desceleration Parameter Model
 * @short_description: FIXME
 *
 * FIXME
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */
#include <numcosmo/numcosmo.h>

#include <glib.h>

G_DEFINE_TYPE (NcHICosmoQSpline, nc_hicosmo_qspline, NC_TYPE_MODEL);

#define VECTOR     (model->params)
#define QSPLINE_H0 (ncm_vector_get (VECTOR, NC_HICOSMO_QSPLINE_H0))
#define OMEGA_T    (ncm_vector_get (VECTOR, NC_HICOSMO_QSPLINE_OMEGA_T))

static gdouble
_nc_hicosmo_qspline_dE2dz (gdouble E2, gdouble z, gpointer userdata)
{
  NcHICosmoQSpline *qs = NC_HICOSMO_QSPLINE (userdata);
  gdouble q;
  if (z > qs->z_f)
	q = ncm_spline_eval (qs->q_z, qs->z_f);
  else
	q = ncm_spline_eval (qs->q_z, z);
  return 2.0 * E2 * (q + 1.0) / (1.0 + z);
}

static void
_nc_hicosmo_qspline_prepare (NcHICosmoQSpline *qs)
{
  if (NCM_MODEL (qs)->pkey > qs->pkey)
  {
	ncm_spline_prepare (qs->q_z);
	ncm_ode_spline_prepare (qs->E2_z, qs);
	qs->pkey = NCM_MODEL (qs)->pkey;
  }
  else
	return;
}

static gdouble
_nc_hicosmo_qspline_E2 (NcmModel *model, gdouble z)
{
  NcHICosmoQSpline *qs = NC_HICOSMO_QSPLINE (model);
  _nc_hicosmo_qspline_prepare (qs);
  if (z > qs->z_f)
  {
	gdouble q = ncm_spline_eval (qs->q_z, qs->z_f);
	return ncm_spline_eval (qs->E2_z->s, qs->z_f) * pow ((1.0 + z) / (1.0 + qs->z_f), 2.0 * (q + 1.0));
  }
  else
	return ncm_spline_eval (qs->E2_z->s, z);
}

static gdouble
_nc_hicosmo_qspline_dE2_dz (NcmModel *model, gdouble z)
{
  NcHICosmoQSpline *qs = NC_HICOSMO_QSPLINE (model);
  _nc_hicosmo_qspline_prepare (qs);
  if (z > qs->z_f)
  {
	gdouble q = ncm_spline_eval (qs->q_z, qs->z_f);
	gdouble x = 1.0 + z;
	return ncm_spline_eval (qs->E2_z->s, qs->z_f) *
	  pow (x / (1.0 + qs->z_f), 2.0 * (q + 1.0)) *
	  2.0 * (q + 1.0) / x;
  }
  else
	return ncm_spline_eval_deriv (qs->E2_z->s, z);
}

static gdouble
_nc_hicosmo_qspline_d2E2_dz2 (NcmModel *model, gdouble z)
{
  NcHICosmoQSpline *qs = NC_HICOSMO_QSPLINE (model);
  _nc_hicosmo_qspline_prepare (qs);
  if (z > qs->z_f)
  {
	gdouble q = ncm_spline_eval (qs->q_z, qs->z_f);
	gdouble x = 1.0 + z;
	gdouble x2 = x * x;
	return ncm_spline_eval (qs->E2_z->s, qs->z_f) *
	  pow (x / (1.0 + qs->z_f), 2.0 * (q + 1.0)) *
	  2.0 * (q + 1.0) * (2.0 * q + 1.0) / x2;
  }
  else
	return ncm_spline_eval_deriv2 (qs->E2_z->s, z);
}

/****************************************************************************
 * Hubble constant
 ****************************************************************************/
static gdouble _nc_hicosmo_qspline_H0 (NcmModel *model) { return QSPLINE_H0; }
static gdouble _nc_hicosmo_qspline_Omega_t (NcmModel *model) { return OMEGA_T; }

/**
 * FIXME
 */
NcHICosmoQSpline *
nc_hicosmo_qspline_new (NcmSpline *s, gsize np, gdouble z_f)
{
  NcHICosmoQSpline *qspline = g_object_new (NC_TYPE_MODEL_QSPLINE,
                                          "spline", s,
                                          "zf", z_f,
                                          "q-length", np,
                                          NULL);

  return qspline;
}

enum {
  PROP_0,
  PROP_SPLINE,
  PROP_NKNOTS,
  PROP_Z_F,
  PROP_SIZE,
};

static void
nc_hicosmo_qspline_init (NcHICosmoQSpline *qspline)
{
  qspline->nknots = 0;
  qspline->size = 0;
  qspline->z_f = 0.0;
  qspline->q_z = NULL;
  qspline->E2_z = NULL;
  qspline->pkey = NCM_MODEL (qspline)->pkey;
}

static void
_nc_hicosmo_qspline_constructed (GObject *object)
{
  /* Chain up : start */
  G_OBJECT_CLASS (nc_hicosmo_qspline_parent_class)->constructed (object);
  {
	NcHICosmoQSpline *qspline = NC_HICOSMO_QSPLINE (object);
	NcmModel *model = NCM_MODEL (qspline);
	NcmModelClass *model_class = NCM_MODEL_GET_CLASS (model);
	NcmVector *zv, *qv;
	guint i;

	qspline->size = model_class->sparam_len + qspline->nknots;
	model->params = ncm_vector_new (qspline->size);

	zv = ncm_vector_new (qspline->nknots);
	qv = ncm_vector_new (qspline->nknots);

	for (i = 0; i < qspline->nknots; i++)
	{
	  gdouble zi = qspline->z_f / (qspline->nknots - 1.0) * i;
	  gdouble xi = zi + 1.0;
	  gdouble xi2 = xi * xi;
	  gdouble xi3 = xi2 * xi;
	  gdouble xi4 = xi2 * xi2;
	  gdouble Ei2 = 1e-5 * xi4 + 0.25 * xi3 + (0.75 - 1e-5);
	  gdouble qi = (1e-5 * xi4 + 0.25 * xi3 * 0.5 - (0.75 - 1e-5)) / Ei2;
	  ncm_vector_set (zv, i, zi);
	  ncm_vector_set (model->params, NC_HICOSMO_QSPLINE_Q + i, qi);
	}

	ncm_spline_set (qspline->q_z, zv, qv, FALSE);
	{
	  NcmSpline *s = ncm_spline_cubic_notaknot_new ();
	  qspline->E2_z = ncm_ode_spline_new (s,
	                                      _nc_hicosmo_qspline_dE2dz, qspline,
	                                      1.0, 0.0, qspline->z_f);
	  ncm_spline_free (s);
	}

	return;
  }
}

static void
_nc_hicosmo_qspline_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  NcHICosmoQSpline *qspline = NC_HICOSMO_QSPLINE (object);

  g_return_if_fail (NC_IS_MODEL_QSPLINE (object));

  switch (prop_id)
  {
	case PROP_SPLINE:
	  g_value_set_object (value, qspline->q_z);
	  break;
	case PROP_NKNOTS:
	  g_value_set_uint (value, qspline->nknots);
	  break;
	case PROP_Z_F:
	  g_value_set_double (value, qspline->z_f);
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  break;
  }
}

static void
_nc_hicosmo_qspline_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  NcHICosmoQSpline *qspline = NC_HICOSMO_QSPLINE (object);
  g_return_if_fail (NC_IS_MODEL_QSPLINE (object));

  switch (prop_id)
  {
	case PROP_SPLINE:
	  qspline->q_z = g_value_get_object (value);
	  break;
	case PROP_NKNOTS:
	  qspline->nknots = g_value_get_uint (value);
	  break;
	case PROP_Z_F:
	  qspline->z_f = g_value_get_double (value);
	  break;
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  break;
  }
}

static void
nc_hicosmo_qspline_dispose (GObject *object)
{
  NcHICosmoQSpline *qspline = NC_HICOSMO_QSPLINE (object);
  ncm_spline_free (qspline->q_z);
  ncm_ode_spline_free (qspline->E2_z);

  /* Chain up : end */
  G_OBJECT_CLASS (nc_hicosmo_qspline_parent_class)->dispose (object);
}

static void
nc_hicosmo_qspline_finalize (GObject *object)
{

  /* Chain up : end */
  G_OBJECT_CLASS (nc_hicosmo_qspline_parent_class)->finalize (object);
}

static void
nc_hicosmo_qspline_class_init (NcHICosmoQSplineClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  NcHICosmoClass* parent_class = NC_HICOSMO_CLASS (klass);
  NcmModelClass *model_class = NCM_MODEL_CLASS (klass);

  object_class->set_property = &ncm_model_class_set_property;
  object_class->get_property = &ncm_model_class_get_property;
  object_class->constructed  = &_nc_hicosmo_qspline_constructed;
  object_class->dispose      = &nc_hicosmo_qspline_dispose;
  object_class->finalize     = &nc_hicosmo_qspline_finalize;

  model_class->set_property = &_nc_hicosmo_qspline_set_property;
  model_class->get_property = &_nc_hicosmo_qspline_get_property;

  ncm_model_class_add_params (model_class, NC_HICOSMO_QSPLINE_SPARAM_LEN, NC_HICOSMO_QSPLINE_VPARAM_LEN, PROP_SIZE);
  ncm_model_class_set_name_nick (model_class, g_strdup ("Q Spline"), g_strdup ("qspline"));

  ncm_model_class_set_sparam (model_class, NC_HICOSMO_QSPLINE_H0, "H_0", "H0",
                                     10.0, 500.0, 1.0, NC_HICOSMO_DEFAULT_PARAMS_ABSTOL, NC_HICOSMO_QSPLINE_DEFAULT_H0,
                                     NCM_PARAM_TYPE_FIXED);

  ncm_model_class_set_sparam (model_class, NC_HICOSMO_QSPLINE_OMEGA_T, "\\Omega_t", "Omegat",
                                     -5.0, 5.0, 1.0e-1,
                                     NC_HICOSMO_DEFAULT_PARAMS_ABSTOL, NC_HICOSMO_QSPLINE_DEFAULT_OMEGA_T,
                                     NCM_PARAM_TYPE_FIXED);

  ncm_model_class_set_vparam (model_class, NC_HICOSMO_QSPLINE_Q, NC_HICOSMO_QSPLINE_DEFAULT_Q_LEN, "q", "q",
                                     -50.0, 50.0, 1.0e-1, NC_HICOSMO_DEFAULT_PARAMS_ABSTOL, NC_HICOSMO_QSPLINE_DEFAULT_Q,
                                     NCM_PARAM_TYPE_FREE);

  /* Check for errors in parameters initialization */
  ncm_model_class_check_params_info (model_class);

  g_object_class_install_property (object_class,
                                   PROP_SPLINE,
                                   g_param_spec_object ("spline",
                                                        NULL,
                                                        "Spline object",
                                                        NCM_TYPE_SPLINE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_Z_F,
                                   g_param_spec_double ("zf",
                                                        NULL,
                                                        "final redshift",
                                                        0.0, 100.0, 1.0,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));

  nc_hicosmo_set_H0_impl       (parent_class, &_nc_hicosmo_qspline_H0);
  nc_hicosmo_set_E2_impl       (parent_class, &_nc_hicosmo_qspline_E2);
  nc_hicosmo_set_dE2_dz_impl   (parent_class, &_nc_hicosmo_qspline_dE2_dz);
  nc_hicosmo_set_d2E2_dz2_impl (parent_class, &_nc_hicosmo_qspline_d2E2_dz2);
  nc_hicosmo_set_Omega_t_impl  (parent_class, &_nc_hicosmo_qspline_Omega_t);
}