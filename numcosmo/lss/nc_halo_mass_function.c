/***************************************************************************
 *            nc_halo_mass_function.c
 *
 *  Mon Jun 28 15:09:13 2010
 *  Copyright  2010  Mariana Penna Lima
 *  <pennalima@gmail.com>
 ****************************************************************************/
/*
 * numcosmo
 * Copyright (C) Mariana Penna Lima 2012 <pennalima@gmail.com>
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
 * SECTION:nc_halo_mass_function
 * @title: NcMassFunction
 * @short_description: Clusters mass function.
 *
 * FIXME
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include "build_cfg.h"

#include "lss/nc_halo_mass_function.h"
#include "math/integral.h"
#include "math/ncm_spline2d_bicubic.h"
#include "math/ncm_cfg.h"

#include <gsl/gsl_integration.h>

enum
{
  PROP_0,
  PROP_DISTANCE,
  PROP_MATTER_VAR,
  PROP_GROWTH,
  PROP_MULTIPLICITY,
  PROP_AREA,
  PROP_PREC,
};

G_DEFINE_TYPE (NcMassFunction, nc_halo_mass_function, G_TYPE_OBJECT);

static void
nc_halo_mass_function_init (NcMassFunction *mfp)
{
  mfp->lnMi        = 0.0;
  mfp->lnMf        = 0.0;
  mfp->zi          = 0.0;
  mfp->zf          = 0.0;
  mfp->area_survey = 0.0;
  mfp->N_sigma     = 0.0;
  mfp->growth      = 0.0;
  mfp->d2NdzdlnM   = NULL;
  mfp->prec        = 0.0;
  mfp->ctrl_cosmo  = ncm_model_ctrl_new (NULL);
  mfp->ctrl_reion  = ncm_model_ctrl_new (NULL);
}

static void
_nc_halo_mass_function_dispose (GObject *object)
{
  NcMassFunction *mfp = NC_HALO_MASS_FUNCTION (object);

  nc_distance_clear (&mfp->dist);
  nc_matter_var_clear (&mfp->vp);
  nc_growth_func_clear (&mfp->gf);
  nc_multiplicity_func_clear (&mfp->mulf);

  ncm_model_ctrl_clear (&mfp->ctrl_cosmo);
  ncm_model_ctrl_clear (&mfp->ctrl_reion);

  ncm_spline2d_clear (&mfp->d2NdzdlnM);

  /* Chain up : end */
  G_OBJECT_CLASS (nc_halo_mass_function_parent_class)->dispose (object);
}

static void
_nc_halo_mass_function_finalize (GObject *object)
{

  /* Chain up : end */
  G_OBJECT_CLASS (nc_halo_mass_function_parent_class)->finalize (object);
}

static void
_nc_halo_mass_function_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  NcMassFunction *mfp = NC_HALO_MASS_FUNCTION (object);
  g_return_if_fail (NC_IS_HALO_MASS_FUNCTION (object));

  switch (prop_id)
  {
    case PROP_DISTANCE:
      mfp->dist = g_value_dup_object (value);
      break;
    case PROP_MATTER_VAR:
      mfp->vp = g_value_dup_object (value);
      break;
    case PROP_GROWTH:
      mfp->gf = g_value_dup_object (value);
      break;
    case PROP_MULTIPLICITY:
      mfp->mulf = g_value_dup_object (value);
      break;
    case PROP_AREA:
      nc_halo_mass_function_set_area (mfp, g_value_get_double (value));
      break;
    case PROP_PREC:
      nc_halo_mass_function_set_prec (mfp, g_value_get_double (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
_nc_halo_mass_function_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  NcMassFunction *mfp = NC_HALO_MASS_FUNCTION (object);
  g_return_if_fail (NC_IS_HALO_MASS_FUNCTION (object));

  switch (prop_id)
  {
    case PROP_DISTANCE:
      g_value_set_object (value, mfp->dist);
      break;
    case PROP_MATTER_VAR:
      g_value_set_object (value, mfp->vp);
      break;
    case PROP_GROWTH:
      g_value_set_object (value, mfp->gf);
      break;
    case PROP_MULTIPLICITY:
      g_value_set_object (value, mfp->mulf);
      break;
    case PROP_AREA:
      g_value_set_double (value, mfp->area_survey);
      break;
    case PROP_PREC:
      g_value_set_double (value, mfp->prec);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
nc_halo_mass_function_class_init (NcMassFunctionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  //GObjectClass* parent_class = G_OBJECT_CLASS (klass);

  object_class->dispose = _nc_halo_mass_function_dispose;
  object_class->finalize = _nc_halo_mass_function_finalize;
  object_class->set_property = _nc_halo_mass_function_set_property;
  object_class->get_property = _nc_halo_mass_function_get_property;

  /**
   * NcMassFunction:distance:
   *
   * This property keeps the distance object.
   */
  g_object_class_install_property (object_class,
                                   PROP_DISTANCE,
                                   g_param_spec_object ("distance",
                                                        NULL,
                                                        "Distance.",
                                                        NC_TYPE_DISTANCE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                                        | G_PARAM_STATIC_BLURB));

  /**
   * NcMassFunction:variance:
   *
   * This property keeps the matter variance object.
   */
  g_object_class_install_property (object_class,
                                   PROP_MATTER_VAR,
                                   g_param_spec_object ("variance",
                                                        NULL,
                                                        "Variance.",
                                                        NC_TYPE_MATTER_VAR,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                                        | G_PARAM_STATIC_BLURB));
  /**
   * NcMassFunction:growth:
   *
   * This property keeps the growth function object.
   */
  g_object_class_install_property (object_class,
                                   PROP_GROWTH,
                                   g_param_spec_object ("growth",
                                                        NULL,
                                                        "Growth function.",
                                                        NC_TYPE_GROWTH_FUNC,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                                        | G_PARAM_STATIC_BLURB));
  /**
   * NcMassFunction:multiplicity:
   *
   * This property keeps the multiplicity function object.
   */
  g_object_class_install_property (object_class,
                                   PROP_MULTIPLICITY,
                                   g_param_spec_object ("multiplicity",
                                                        NULL,
                                                        "Multiplicity function.",
                                                        NC_TYPE_MULTIPLICITY_FUNC,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME
                                                        | G_PARAM_STATIC_BLURB));

  /**
   * NcMassFunction:area:
   *
   * This property sets the angular area in steradian.
   */
  g_object_class_install_property (object_class,
                                   PROP_AREA,
                                   g_param_spec_double ("area",
                                                        NULL,
                                                        "Angular area in steradian",
                                                        0.0, 4.0 * M_PI, 200.0 * M_PI * M_PI / (180.0 * 180.0),
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME
                                                        | G_PARAM_STATIC_BLURB));

  /**
   * NcMassFunction:prec:
   *
   * This property sets the precision.
   */
  g_object_class_install_property (object_class,
                                   PROP_PREC,
                                   g_param_spec_double ("prec",
                                                        NULL,
                                                        "Precision",
                                                        1.0e-15, 1.0, 1.0e-6,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME
                                                        | G_PARAM_STATIC_BLURB));
}


/**
 * nc_halo_mass_function_new:
 * @dist: a #NcDistance sets to #NcMassFunction:distance
 * @vp: a #NcMatterVar sets to #NcMassFunction:variance
 * @gf: a #NcGrowthFunc sets to #NcMassFunction:growth
 * @mulf: a #NcMultiplicityFunc sets to #NcMassFunction:multiplicity
 *
 * This function allocates memory for a new #NcMassFunction object and sets its properties to the values from
 * the input arguments.
 *
 * Returns: A new #NcMassFunction.
 */
NcMassFunction *
nc_halo_mass_function_new (NcDistance *dist, NcMatterVar *vp, NcGrowthFunc *gf, NcMultiplicityFunc *mulf)
{
  NcMassFunction *mfp = g_object_new (NC_TYPE_HALO_MASS_FUNCTION,
                                      "distance", dist,
                                      "variance", vp,
                                      "growth", gf,
                                      "multiplicity", mulf,
                                      NULL);
  return mfp;
}

/**
 * nc_halo_mass_function_copy:
 * @mfp: a #NcMassFunction
 *
 * This function duplicates the #NcMassFunction object setting the same values of the original propertities.
 *
 * Returns: (transfer full): A new #NcMassFunction.
   */
NcMassFunction *
nc_halo_mass_function_copy (NcMassFunction *mfp)
{
  return nc_halo_mass_function_new (mfp->dist, mfp->vp, mfp->gf, mfp->mulf);
}

/**
 * nc_halo_mass_function_free:
 * @mfp: a #NcMassFunction
 *
 * Atomically decrements the reference count of @mfp by one. If the reference count drops to 0,
 * all memory allocated by @mfp is released.
 *
 */
void
nc_halo_mass_function_free (NcMassFunction *mfp)
{
  g_object_unref (mfp);
}

/**
 * nc_halo_mass_function_clear:
 * @mfp: a #NcMassFunction
 *
 * Atomically decrements the reference count of @mfp by one. If the reference count drops to 0,
 * all memory allocated by @mfp is released. Set pointer to NULL.
 *
 */
void
nc_halo_mass_function_clear (NcMassFunction **mfp)
{
  g_clear_object (mfp);
}


/**
 * nc_halo_mass_function_dn_dlnm:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @lnM: logarithm base e of mass
 * @z: redshift
 *
 * This function computes the comoving number density of dark matter halos at redshift @z and
 * mass M.
 *
 * Returns: $\frac{dn}{dlnM}$
 */
gdouble
nc_halo_mass_function_dn_dlnm (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble lnM, gdouble z)
{
  gdouble lnR          = nc_matter_var_lnM_to_lnR (mfp->vp, cosmo, lnM);
  gdouble R            = exp (lnR);
  gdouble M_rho        = nc_window_volume (mfp->vp->wp) * gsl_pow_3 (R);	/* M/\rho comoving, rho independe de z */
  gdouble var0         = nc_matter_var_var0 (mfp->vp, cosmo, lnR);
  gdouble dlnvar0_dlnR = nc_matter_var_dlnvar0_dlnR (mfp->vp, cosmo, lnR);
  gdouble growth       = nc_growth_func_eval (mfp->gf, cosmo, z);
  gdouble sigma        = nc_matter_var_sigma8_sqrtvar0 (mfp->vp, cosmo) * sqrt (var0) * growth;
  gdouble f            = nc_multiplicity_func_eval (mfp->mulf, cosmo, sigma, z);
  gdouble dn_dlnM      = -(1.0 / (3.0 * M_rho)) * f * 0.5 * dlnvar0_dlnR;	/* dn/dlnM = - (\rho/3M) * f * (R/\sigma)* dsigma_dR */
  
  return dn_dlnM;
}

/**
 * nc_halo_mass_function_sigma:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @lnM: logarithm base e of mass
 * @z: redshift
 * @dn_dlnM_ptr: pointer to comoving number density of halos
 * @sigma_ptr: pointer to the standard deviation of the density contrast
 *
 * This function computes the standard deviation of density contrast of the matter fluctuations and
 * the the comoving number density of dark matter halos at redshift @z and mass M.
 * These values are stored in @sigma_ptr and @dn_dlnM_ptr, respectively.
 *
 */
void
nc_halo_mass_function_sigma (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble lnM, gdouble z, gdouble *dn_dlnM_ptr, gdouble *sigma_ptr)
{
  const gdouble lnR = nc_matter_var_lnM_to_lnR (mfp->vp, cosmo, lnM);
  const gdouble M_rho = nc_window_volume (mfp->vp->wp) * gsl_pow_3 (exp (lnR));	/* M/\rho comoving, rho independe de z */
  const gdouble growth = nc_growth_func_eval (mfp->gf, cosmo, z);
  const gdouble sigma8_sqrtvar0 = nc_matter_var_sigma8_sqrtvar0 (mfp->vp, cosmo);
  const gdouble var0 = nc_matter_var_var0 (mfp->vp, cosmo, lnR);
  const gdouble dlnvar0_dlnR = nc_matter_var_dlnvar0_dlnR (mfp->vp, cosmo, lnR);
  const gdouble sigma = sigma8_sqrtvar0 * sqrt (var0) * growth;
  const gdouble f = nc_multiplicity_func_eval (mfp->mulf, cosmo, sigma, z);

  *sigma_ptr = sigma;
  *dn_dlnM_ptr = -(1.0 / (3.0 * M_rho)) * f * (1.0 / 2.0) * dlnvar0_dlnR;	/* dn/dlnM = - (\rho/3M) * f * (R/\sigma)* dsigma_dR */

  return;
}

/* A funcao abaixo foi feita para compararmos o nosso calculo com o resultado (equacao 9) do artigo astro-ph/0110246.*/
/**
 * nc_halo_mass_function_alpha_eff:
 * @vp: a #NcMatterVar
 * @cosmo: a #NcHICosmo
 * @lnM: logarithm base e of mass
 * @a_eff_ptr: FIXME
 *
 * FIXME
 *
 */
void
nc_halo_mass_function_alpha_eff (NcMatterVar *vp, NcHICosmo *cosmo, gdouble lnM, gdouble *a_eff_ptr)
{
  gdouble lnR = nc_matter_var_lnM_to_lnR (vp, cosmo, lnM);
  gdouble dlnvar0_dlnR = nc_matter_var_dlnvar0_dlnR (vp, cosmo, lnR);
  *a_eff_ptr = -(1.0 / 3.0) * (1.0 / 2.0) * dlnvar0_dlnR;	/* (M/rho)*(1/f)*dn/dlnM = - (1/3) * (1/\sigma)* dsigma_dlnR */

  return;
}

typedef struct _nc_ca_integ
{
  NcMassFunction *mfp;
  NcHICosmo *cosmo;
  gdouble z;
} nc_ca_integ;

/**
 * nc_halo_mass_function_cluster_abundance_integrand:
 * @R: FIXME
 * @params: FIXME
 * FIXME
 *
 * Returns: FIXME
 */
gdouble
nc_halo_mass_function_cluster_abundance_integrand (gdouble R, gpointer params)
{
  nc_ca_integ *ca_integ = (nc_ca_integ *) params;
  NcHICosmo *cosmo = ca_integ->cosmo;
  NcMassFunction *mfp = ca_integ->mfp;
  const gdouble lnR = log (R);
  {
    const gdouble M_rho = nc_window_volume (mfp->vp->wp) * gsl_pow_3 (R);
    const gdouble var0 = nc_matter_var_var0 (mfp->vp, cosmo, lnR);
    const gdouble dlnvar0_dR = nc_matter_var_dlnvar0_dR (mfp->vp, cosmo, lnR);
    gdouble sigma, f, d2cadvdR;

    sigma = mfp->N_sigma * sqrt (var0) * mfp->growth;
    f = nc_multiplicity_func_eval (mfp->mulf, cosmo, sigma, ca_integ->z);
    d2cadvdR = -(1.0 / M_rho) * f * (1.0 / 2.0) * dlnvar0_dR;

    return d2cadvdR;
  }
}

/**
 * nc_halo_mass_function_dcluster_abundance_dv:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @M: mass in units of h^{-1} * M_sun
 * @z: redshift
 *
 * FIXME
 *
 * Returns: FIXME
 */
gdouble
nc_halo_mass_function_dn_m_to_inf_dv (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble M, gdouble z)	/* integracao em M. */
{
  static gsl_integration_workspace *w = NULL;
  gsl_function F;
  gdouble n, n_part, error, R;
  nc_ca_integ ca_integ;

  ca_integ.mfp = mfp;
  ca_integ.cosmo = cosmo;
  ca_integ.z = z;
  F.function = &nc_halo_mass_function_cluster_abundance_integrand;
  F.params = &ca_integ;

  if (w == NULL)
    w = gsl_integration_workspace_alloc (NCM_INTEGRAL_PARTITION);

  n = 0.0;
  R = nc_matter_var_mass_to_R (mfp->vp, cosmo, M);
  mfp->growth = nc_growth_func_eval (mfp->gf, cosmo, z);

#define _NC_STEP 20.0

  while (1)
  {
    gsl_integration_qag (&F, R, R + _NC_STEP, 0.0, NCM_DEFAULT_PRECISION, NCM_INTEGRAL_PARTITION, 1, w, &n_part, &error);
    R += _NC_STEP;
    n += n_part;

    if (gsl_fcmp (n, n + n_part, NCM_DEFAULT_PRECISION * 1e-1) == 0)
      break;
  }

  return n;
}

/**
 * nc_halo_mass_function_dn_m1_to_m2_dv:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @M1: mass in units of h^{-1} M_sun - lower limit of integration
 * @M2: mass in units of h^{-1} M_sun- upper limit of integration
 * @z: redshift
 *
 * FIXME
 *
 * Returns: FIXME
 */
gdouble
nc_halo_mass_function_dn_m1_to_m2_dv (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble M1, gdouble M2, gdouble z)	/* integracao em M. */
{
  static gsl_integration_workspace *w = NULL;
  gsl_function F;
  gdouble n, error, R1, R2;
  nc_ca_integ ca_integ;

  ca_integ.mfp = mfp;
  ca_integ.cosmo = cosmo;
  ca_integ.z = z;
  F.function = &nc_halo_mass_function_cluster_abundance_integrand;
  F.params = &ca_integ;

  if (w == NULL)
    w = gsl_integration_workspace_alloc (NCM_INTEGRAL_PARTITION);

  n = 0.0;
  R1 = nc_matter_var_mass_to_R (mfp->vp, cosmo, M1);
  R2 = nc_matter_var_mass_to_R (mfp->vp, cosmo, M2);
  mfp->growth = nc_growth_func_eval (mfp->gf, cosmo, z);

  gsl_integration_qag (&F, R1, R2, 0.0, NCM_DEFAULT_PRECISION, NCM_INTEGRAL_PARTITION, 1, w, &n, &error);

  /*printf ("R1 = %5.5g R2 = %5.5g z = %5.5g n = %5.5g\n", R1, R2, z, n);*/
  return n;
}

/**
 * nc_halo_mass_function_dv_dzdomega:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @z: redshift
 *
 * This function computes the comoving volume (flat universe) element per unit solid angle $d\Omega$ 
 * given @z, namely, $$\frac{d^2V}{dzd\Omega} = \frac{c}{H(z)} D_c^2(z),$$
 * where $H(z)$ is the Hubble function and $D_c$ is the comoving distance.
 *
 * Returns: comoving volume element $d^2V / dzd\Omega$.
 */
gdouble
nc_halo_mass_function_dv_dzdomega (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble z)
{
  const gdouble E = sqrt (nc_hicosmo_E2 (cosmo, z));
  gdouble dc = ncm_c_hubble_radius () * nc_distance_comoving (mfp->dist, cosmo, z);
  gdouble dV_dzdOmega = gsl_pow_2 (dc) * ncm_c_hubble_radius () / E;
  return dV_dzdOmega;
}

static void _nc_halo_mass_function_generate_2Dspline_knots (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble rel_error);

/**
 * nc_halo_mass_function_set_area:
 * @mfp: a #NcMassFunction
 * @area: area in steradian
 *
 * FIXME
 *
 */
void
nc_halo_mass_function_set_area (NcMassFunction *mfp, gdouble area)
{
  if (mfp->area_survey != area)
  {
    mfp->area_survey = area;
    ncm_spline2d_clear (&mfp->d2NdzdlnM);
  }
}

/**
 * nc_halo_mass_function_set_prec:
 * @mfp: a #NcMassFunction
 * @prec: precision
 *
 * FIXME
 *
 */
void
nc_halo_mass_function_set_prec (NcMassFunction *mfp, gdouble prec)
{
  if (mfp->prec != prec)
  {
    mfp->prec = prec;
    ncm_spline2d_clear (&mfp->d2NdzdlnM);
  }
}

/**
 * nc_halo_mass_function_set_area_sd:
 * @mfp: a #NcMassFunction
 * @area_sd: area in square degree
 *
 * FIXME
 *
 */
void
nc_halo_mass_function_set_area_sd (NcMassFunction *mfp, gdouble area_sd)
{
  const gdouble conversion_factor = M_PI * M_PI / (180.0 * 180.0);
  nc_halo_mass_function_set_area (mfp, area_sd * conversion_factor);
}

/**
 * nc_halo_mass_function_set_eval_limits:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @lnMi: minimum logarithm base e of mass
 * @lnMf: maximum logarithm base e of mass
 * @zi: minimum redshift
 * @zf: maximum redshift
 *
 * FIXME
 *
 */
void
nc_halo_mass_function_set_eval_limits (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble lnMi, gdouble lnMf, gdouble zi, gdouble zf)
{
  if (lnMi != mfp->lnMi || mfp->lnMf != lnMf || mfp->zi != zi || mfp->zf != zf)
  {
    mfp->lnMi = lnMi;
    mfp->lnMf = lnMf;
    mfp->zi = zi;
    mfp->zf = zf;
    ncm_spline2d_clear (&mfp->d2NdzdlnM);
  }
}

/**
 * nc_halo_mass_function_prepare:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 *
 * FIXME
 *
 */
void
nc_halo_mass_function_prepare (NcMassFunction *mfp, NcHICosmo *cosmo)
{
  guint i, j;

  nc_distance_prepare_if_needed (mfp->dist, cosmo);
  nc_matter_var_prepare_if_needed (mfp->vp, cosmo);
  nc_growth_func_prepare_if_needed (mfp->gf, cosmo);
  
  if (mfp->d2NdzdlnM == NULL)
    _nc_halo_mass_function_generate_2Dspline_knots (mfp, cosmo, mfp->prec);

#define D2NDZDLNM_Z(cad) ((cad)->d2NdzdlnM->yv)
#define D2NDZDLNM_LNM(cad) ((cad)->d2NdzdlnM->xv)
#define D2NDZDLNM_VAL(cad) ((cad)->d2NdzdlnM->zm)

  for (i = 0; i < ncm_vector_len (D2NDZDLNM_Z (mfp)); i++)
  {
    const gdouble z = ncm_vector_get (D2NDZDLNM_Z (mfp), i);
    const gdouble dVdz = mfp->area_survey * nc_halo_mass_function_dv_dzdomega (mfp, cosmo, z);

    for (j = 0; j < ncm_vector_len (D2NDZDLNM_LNM (mfp)); j++)
    {
      const gdouble lnM = ncm_vector_get (D2NDZDLNM_LNM (mfp), j);
      const gdouble d2NdzdlnM_ij = (dVdz * nc_halo_mass_function_dn_dlnm (mfp, cosmo, lnM, z));
      ncm_matrix_set (D2NDZDLNM_VAL (mfp), i, j, d2NdzdlnM_ij);
    }
  }
  ncm_spline2d_prepare (mfp->d2NdzdlnM);

  ncm_model_ctrl_update (mfp->ctrl_cosmo, NCM_MODEL (cosmo));
}

/**
 * nc_halo_mass_function_dn_dz:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @lnMl: logarithm base e of mass, lower threshold
 * @lnMu: logarithm base e of mass, upper threshold
 * @z: redshift
 * @spline: whenever to create an intermediary spline of the mass integration
 *
 * FIXME
 *
 * Returns: FIXME
 */
gdouble
nc_halo_mass_function_dn_dz (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble lnMl, gdouble lnMu, gdouble z, gboolean spline)
{
  gdouble dN_dz;

  if (spline)
    dN_dz = ncm_spline2d_integ_dx_spline_val (mfp->d2NdzdlnM, lnMl, lnMu, z);
  else
    dN_dz = ncm_spline2d_integ_dx (mfp->d2NdzdlnM, lnMl, lnMu, z);
  return dN_dz;
}

/**
 * nc_halo_mass_function_n:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @lnMl: logarithm base e of mass, lower threshold
 * @lnMu: logarithm base e of mass, upper threshold
 * @zl: minimum redshift
 * @zu: maximum redshift
 * @spline: whenever to create an intermediary spline of the integration
 *
 * FIXME
 *
 * Returns: FIXME
 */
gdouble
nc_halo_mass_function_n (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble lnMl, gdouble lnMu, gdouble zl, gdouble zu, NcMassFunctionSplineOptimize spline)
{
  gdouble N;

  switch (spline)
  {
    case NC_HALO_MASS_FUNCTION_SPLINE_NONE:
      N = ncm_spline2d_integ_dxdy (mfp->d2NdzdlnM, lnMl, lnMu, zl, zu);
      break;
    case NC_HALO_MASS_FUNCTION_SPLINE_LNM:
      N = ncm_spline2d_integ_dxdy_spline_x (mfp->d2NdzdlnM, lnMl, lnMu, zl, zu);
      break;
    case NC_HALO_MASS_FUNCTION_SPLINE_Z:
      N = ncm_spline2d_integ_dxdy_spline_y (mfp->d2NdzdlnM, lnMl, lnMu, zl, zu);
      break;
    default:
      g_assert_not_reached ();
      break;
  }
  return N;
}

typedef struct __encapsulated_function_args
{
  NcMassFunction *mfp;
  NcHICosmo *cosmo;
  gdouble z;
  gdouble lnM;
  gdouble dVdz;
} _encapsulated_function_args;

static gdouble
_encapsulated_z (gdouble z, gpointer p)
{
  _encapsulated_function_args *args = (_encapsulated_function_args *) p;

  gdouble A = args->mfp->area_survey *
    nc_halo_mass_function_dv_dzdomega (args->mfp, args->cosmo, z) *
    nc_halo_mass_function_dn_dlnm (args->mfp, args->cosmo, args->lnM, z);

  return A;
}

static gdouble
_encapsulated_lnM (gdouble lnM, gpointer p)
{
  _encapsulated_function_args *args = (_encapsulated_function_args *) p;

  gdouble A = args->dVdz * nc_halo_mass_function_dn_dlnm (args->mfp, args->cosmo, lnM, args->z);

  return A;
}

static void
_nc_halo_mass_function_generate_2Dspline_knots (NcMassFunction *mfp, NcHICosmo *cosmo, gdouble rel_error)
{
  gsl_function Fx, Fy;
  _encapsulated_function_args args;
  g_assert (mfp->d2NdzdlnM == NULL);

  args.mfp = mfp;
  args.cosmo = cosmo;
  args.z = (mfp->zf + mfp->zi) / 2.0;
  args.lnM = (mfp->lnMf + mfp->lnMi) / 2.0;
  args.dVdz = mfp->area_survey * nc_halo_mass_function_dv_dzdomega (mfp, cosmo, args.z);

  Fx.function = _encapsulated_lnM;
  Fx.params = &args;

  Fy.function = _encapsulated_z;
  Fy.params = &args;

  mfp->d2NdzdlnM = ncm_spline2d_bicubic_notaknot_new ();
  ncm_spline2d_set_function (mfp->d2NdzdlnM,
                             NCM_SPLINE_FUNCTION_SPLINE, &Fx, &Fy, mfp->lnMi, mfp->lnMf, mfp->zi, mfp->zf, rel_error);
}

/**
 * nc_halo_mass_function_prepare_if_needed:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 *
 * FIXME
 *
 */
/**
 * nc_halo_mass_function_d2n_dzdlnm:
 * @mfp: a #NcMassFunction
 * @cosmo: a #NcHICosmo
 * @lnM: logarithm base e of mass
 * @z: redshift
 *
 * FIXME
 *
 * Returns: FIXME
 */