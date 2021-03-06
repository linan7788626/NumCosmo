/***************************************************************************
 *            nc_cbe.c
 *
 *  Sat October 24 11:56:56 2015
 *  Copyright  2015  Sandro Dias Pinto Vitenti
 *  <sandro@isoftware.com.br>
 ****************************************************************************/
/*
 * nc_cbe.c
 * Copyright (C) 2015 Sandro Dias Pinto Vitenti <sandro@isoftware.com.br>
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
 * SECTION:nc_cbe
 * @title: NcCBE
 * @short_description: CLASS (Cosmic Linear Anisotropy Solving System) backend
 *
 * This object provides an interface for the CLASS code.
 *
 * If you use this object please cite: [Blas (2011) CLASS II][XBlas2011],
 * see also:
 * - [Lesgourgues (2011) CLASS I][XLesgourgues2011],
 * - [Lesgourgues (2011) CLASS III][XLesgourgues2011a],
 * - [Lesgourgues (2011) CLASS IV][XLesgourgues2011b] and
 * - [CLASS website](http://class-code.net/).
 *
 */

/*
 * It must be include before anything else, several symbols clash
 * with the default includes.
 */
#include "class/include/class.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */
#include "build_cfg.h"

#include "nc_hiprim.h"
#include "nc_hireion_camb.h"
#include "model/nc_hicosmo_de.h"
#include "nc_cbe.h"
#include "nc_enum_types.h"
#include "math/ncm_spline_cubic_notaknot.h"
#include "math/ncm_spline2d_bicubic.h"

enum
{
  PROP_0,
  PROP_PREC,
  PROP_TARGET_CLS,
  PROP_CALC_TRANSFER,
  PROP_USE_LENSED_CLS,
  PROP_USE_TENSOR,
  PROP_USE_THERMODYN,
  PROP_SCALAR_LMAX,
  PROP_VECTOR_LMAX,
  PROP_TENSOR_LMAX,
  PROP_MATTER_PK_MAXZ,
  PROP_MATTER_PK_MAXK,
};

struct _NcCBEPrivate
{
  struct background pba;
  struct thermo pth;
  struct perturbs ppt;
  struct transfers ptr;
  struct primordial ppm;
  struct spectra psp;
  struct nonlinear pnl;
  struct lensing ple;
  struct output pop;
};

G_DEFINE_TYPE (NcCBE, nc_cbe, G_TYPE_OBJECT);

static void
nc_cbe_init (NcCBE *cbe)
{
  cbe->priv           = G_TYPE_INSTANCE_GET_PRIVATE (cbe, NC_TYPE_CBE, NcCBEPrivate);
  cbe->prec           = NULL;
  cbe->ctrl_cosmo     = ncm_model_ctrl_new (NULL);
  cbe->ctrl_prim      = ncm_model_ctrl_new (NULL);
  cbe->a              = NULL;

  cbe->target_Cls     = 0;
  cbe->calc_transfer  = FALSE;
  cbe->use_lensed_Cls = FALSE;
  cbe->use_tensor     = FALSE;
  cbe->scalar_lmax    = 0;
  cbe->vector_lmax    = 0;
  cbe->tensor_lmax    = 0;

  cbe->call               = NULL;
  cbe->free               = NULL;
  cbe->allocated          = FALSE;
  cbe->thermodyn_prepared = FALSE;

  cbe->priv->pba.h                    = 0.0;
  cbe->priv->pba.H0                   = 0.0;
  cbe->priv->pba.T_cmb                = 0.0;
  cbe->priv->pba.Omega0_g             = 0.0;
  cbe->priv->pba.Omega0_ur            = 0.0;
  cbe->priv->pba.Omega0_b             = 0.0;
  cbe->priv->pba.Omega0_cdm           = 0.0;
  cbe->priv->pba.Omega0_dcdmdr        = 0.0;
  cbe->priv->pba.Omega0_dcdm          = 0.0;
  cbe->priv->pba.Gamma_dcdm           = 0.0;
  cbe->priv->pba.N_ncdm               = 0;
  cbe->priv->pba.Omega0_ncdm_tot      = 0.0;
  cbe->priv->pba.ksi_ncdm_default     = 0.0;
  cbe->priv->pba.ksi_ncdm             = NULL;
  cbe->priv->pba.T_ncdm_default       = 0.0;
  cbe->priv->pba.T_ncdm               = NULL;
  cbe->priv->pba.deg_ncdm_default     = 0.0;
  cbe->priv->pba.deg_ncdm             = NULL;
  cbe->priv->pba.ncdm_psd_parameters  = NULL;
  cbe->priv->pba.ncdm_psd_files       = NULL;
  cbe->priv->pba.Omega0_scf           = 0.0;
  cbe->priv->pba.attractor_ic_scf     = _FALSE_;
  cbe->priv->pba.scf_parameters       = NULL;
  cbe->priv->pba.scf_parameters_size  = 0;
  cbe->priv->pba.scf_tuning_index     = 0;
  cbe->priv->pba.phi_ini_scf          = 0;
  cbe->priv->pba.phi_prime_ini_scf    = 0;
  cbe->priv->pba.Omega0_k             = 0.0;
  cbe->priv->pba.K                    = 0.0;
  cbe->priv->pba.sgnK                 = 0;
  cbe->priv->pba.Omega0_lambda        = 0.0;
  cbe->priv->pba.Omega0_fld           = 0.0;
  cbe->priv->pba.a_today              = 0.0;
  cbe->priv->pba.w0_fld               = 0.0;
  cbe->priv->pba.wa_fld               = 0.0;
  cbe->priv->pba.cs2_fld              = 0.0;

  /* thermodynamics structure */

  cbe->priv->pth.YHe                      = 0;
  cbe->priv->pth.recombination            = 0;
  cbe->priv->pth.reio_parametrization     = 0;
  cbe->priv->pth.reio_z_or_tau            = 0;
  cbe->priv->pth.z_reio                   = 0.0;
  cbe->priv->pth.tau_reio                 = 0.0;
  cbe->priv->pth.reionization_exponent    = 0.0;
  cbe->priv->pth.reionization_width       = 0.0;
  cbe->priv->pth.helium_fullreio_redshift = 0.0;
  cbe->priv->pth.helium_fullreio_width    = 0.0;

  cbe->priv->pth.binned_reio_num            = 0;
  cbe->priv->pth.binned_reio_z              = NULL;
  cbe->priv->pth.binned_reio_xe             = NULL;
  cbe->priv->pth.binned_reio_step_sharpness = 0.0;

  cbe->priv->pth.annihilation           = 0.0;
  cbe->priv->pth.decay                  = 0.0;
  cbe->priv->pth.annihilation_variation = 0.0;
  cbe->priv->pth.annihilation_z         = 0.0;
  cbe->priv->pth.annihilation_zmax      = 0.0;
  cbe->priv->pth.annihilation_zmin      = 0.0;
  cbe->priv->pth.annihilation_f_halo    = 0.0;
  cbe->priv->pth.annihilation_z_halo    = 0.0;
  cbe->priv->pth.has_on_the_spot        = _FALSE_;

  cbe->priv->pth.compute_cb2_derivatives = _FALSE_;

  /* perturbation structure */

  cbe->priv->ppt.has_perturbations            = _FALSE_;
  cbe->priv->ppt.has_cls                      = _FALSE_;

  cbe->priv->ppt.has_cl_cmb_temperature       = _FALSE_;
  cbe->priv->ppt.has_cl_cmb_polarization      = _FALSE_;
  cbe->priv->ppt.has_cl_cmb_lensing_potential = _FALSE_;
  cbe->priv->ppt.has_cl_number_count          = _FALSE_;
  cbe->priv->ppt.has_cl_lensing_potential     = _FALSE_;
  cbe->priv->ppt.has_pk_matter                = _FALSE_;
  cbe->priv->ppt.has_density_transfers        = _FALSE_;
  cbe->priv->ppt.has_velocity_transfers       = _FALSE_;

  cbe->priv->ppt.has_nl_corrections_based_on_delta_m = _FALSE_;

  cbe->priv->ppt.has_nc_density = _FALSE_;
  cbe->priv->ppt.has_nc_rsd     = _FALSE_;
  cbe->priv->ppt.has_nc_lens    = _FALSE_;
  cbe->priv->ppt.has_nc_gr      = _FALSE_;

  cbe->priv->ppt.switch_sw         = 0;
  cbe->priv->ppt.switch_eisw       = 0;
  cbe->priv->ppt.switch_lisw       = 0;
  cbe->priv->ppt.switch_dop        = 0;
  cbe->priv->ppt.switch_pol        = 0;
  cbe->priv->ppt.eisw_lisw_split_z = 0;

  cbe->priv->ppt.has_ad  = _FALSE_;
  cbe->priv->ppt.has_bi  = _FALSE_;
  cbe->priv->ppt.has_cdi = _FALSE_;
  cbe->priv->ppt.has_nid = _FALSE_;
  cbe->priv->ppt.has_niv = _FALSE_;

  cbe->priv->ppt.has_perturbed_recombination = _FALSE_;
  cbe->priv->ppt.tensor_method               = tm_massless_approximation;
  cbe->priv->ppt.evolve_tensor_ur            = _FALSE_;
  cbe->priv->ppt.evolve_tensor_ncdm          = _FALSE_;

  cbe->priv->ppt.has_scalars = _FALSE_;
  cbe->priv->ppt.has_vectors = _FALSE_;
  cbe->priv->ppt.has_tensors = _FALSE_;

  cbe->priv->ppt.l_scalar_max = 0;
  cbe->priv->ppt.l_vector_max = 0;
  cbe->priv->ppt.l_tensor_max = 0;
  cbe->priv->ppt.l_lss_max    = 0;
  cbe->priv->ppt.k_max_for_pk = 0.0;

  cbe->priv->ppt.gauge = synchronous;

  cbe->priv->ppt.k_output_values_num     = 0;
  cbe->priv->ppt.store_perturbations     = _FALSE_;
  cbe->priv->ppt.number_of_scalar_titles = 0;
  cbe->priv->ppt.number_of_vector_titles = 0;
  cbe->priv->ppt.number_of_tensor_titles = 0;
  {
    guint filenum;
    for (filenum = 0; filenum<_MAX_NUMBER_OF_K_FILES_; filenum++){
      cbe->priv->ppt.scalar_perturbations_data[filenum] = NULL;
      cbe->priv->ppt.vector_perturbations_data[filenum] = NULL;
      cbe->priv->ppt.tensor_perturbations_data[filenum] = NULL;
    }
  }
  cbe->priv->ppt.index_k_output_values = NULL;

  /* primordial structure */

  cbe->priv->ppm.primordial_spec_type = analytic_Pk;
  cbe->priv->ppm.k_pivot              = 0.0;
  cbe->priv->ppm.A_s                  = 0.0;
  cbe->priv->ppm.n_s                  = 0.0;
  cbe->priv->ppm.alpha_s              = 0.0;
  cbe->priv->ppm.f_bi                 = 0.0;
  cbe->priv->ppm.n_bi                 = 0.0;
  cbe->priv->ppm.alpha_bi             = 0.0;
  cbe->priv->ppm.f_cdi                = 0.0;
  cbe->priv->ppm.n_cdi                = 0.0;
  cbe->priv->ppm.alpha_cdi            = 0.0;
  cbe->priv->ppm.f_nid                = 0.0;
  cbe->priv->ppm.n_nid                = 0.0;
  cbe->priv->ppm.alpha_nid            = 0.0;
  cbe->priv->ppm.f_niv                = 0.0;
  cbe->priv->ppm.n_niv                = 0.0;
  cbe->priv->ppm.alpha_niv            = 0.0;
  cbe->priv->ppm.c_ad_bi              = 0.0;
  cbe->priv->ppm.n_ad_bi              = 0.0;
  cbe->priv->ppm.alpha_ad_bi          = 0.0;
  cbe->priv->ppm.c_ad_cdi             = 0.0;
  cbe->priv->ppm.n_ad_cdi             = 0.0;
  cbe->priv->ppm.alpha_ad_cdi         = 0.0;
  cbe->priv->ppm.c_ad_nid             = 0.0;
  cbe->priv->ppm.n_ad_nid             = 0.0;
  cbe->priv->ppm.alpha_ad_nid         = 0.0;
  cbe->priv->ppm.c_ad_niv             = 0.0;
  cbe->priv->ppm.n_ad_niv             = 0.0;
  cbe->priv->ppm.alpha_ad_niv         = 0.0;
  cbe->priv->ppm.c_bi_cdi             = 0.0;
  cbe->priv->ppm.n_bi_cdi             = 0.0;
  cbe->priv->ppm.alpha_bi_cdi         = 0.0;
  cbe->priv->ppm.c_bi_nid             = 0.0;
  cbe->priv->ppm.n_bi_nid             = 0.0;
  cbe->priv->ppm.alpha_bi_nid         = 0.0;
  cbe->priv->ppm.c_bi_niv             = 0.0;
  cbe->priv->ppm.n_bi_niv             = 0.0;
  cbe->priv->ppm.alpha_bi_niv         = 0.0;
  cbe->priv->ppm.c_cdi_nid            = 0.0;
  cbe->priv->ppm.n_cdi_nid            = 0.0;
  cbe->priv->ppm.alpha_cdi_nid        = 0.0;
  cbe->priv->ppm.c_cdi_niv            = 0.0;
  cbe->priv->ppm.n_cdi_niv            = 0.0;
  cbe->priv->ppm.alpha_cdi_niv        = 0.0;
  cbe->priv->ppm.c_nid_niv            = 0.0;
  cbe->priv->ppm.n_nid_niv            = 0.0;
  cbe->priv->ppm.alpha_nid_niv        = 0.0;
  cbe->priv->ppm.r                    = 0.0;
  cbe->priv->ppm.n_t                  = 0.0;
  cbe->priv->ppm.alpha_t              = 0.0;
  cbe->priv->ppm.potential            = 0;
  cbe->priv->ppm.phi_end              = 0.0;
  cbe->priv->ppm.ln_aH_ratio          = 0;
  cbe->priv->ppm.V0                   = 0.0;
  cbe->priv->ppm.V1                   = 0.0;
  cbe->priv->ppm.V2                   = 0.0;
  cbe->priv->ppm.V3                   = 0.0;
  cbe->priv->ppm.V4                   = 0.0;
  cbe->priv->ppm.H0                   = 0.0;
  cbe->priv->ppm.H1                   = 0.0;
  cbe->priv->ppm.H2                   = 0.0;
  cbe->priv->ppm.H3                   = 0.0;
  cbe->priv->ppm.H4                   = 0.0;
  cbe->priv->ppm.command              = NULL;
  cbe->priv->ppm.custom1              = 0.0;
  cbe->priv->ppm.custom2              = 0.0;
  cbe->priv->ppm.custom3              = 0.0;
  cbe->priv->ppm.custom4              = 0.0;
  cbe->priv->ppm.custom5              = 0.0;
  cbe->priv->ppm.custom6              = 0.0;
  cbe->priv->ppm.custom7              = 0.0;
  cbe->priv->ppm.custom8              = 0.0;
  cbe->priv->ppm.custom9              = 0.0;
  cbe->priv->ppm.custom10             = 0.0;

  /* transfer structure */

  cbe->priv->ppt.selection_num        = 0;
  cbe->priv->ppt.selection            = 0;
  cbe->priv->ppt.selection_mean[0]    = 0.0;
  cbe->priv->ppt.selection_width[0]   = 0.0;

  cbe->priv->ptr.lcmb_rescale         = 0.0;
  cbe->priv->ptr.lcmb_pivot           = 0.0;
  cbe->priv->ptr.lcmb_tilt            = 0.0;
  cbe->priv->ptr.initialise_HIS_cache = _FALSE_;
  cbe->priv->ptr.has_nz_analytic      = _FALSE_;
  cbe->priv->ptr.has_nz_file          = _FALSE_;
  cbe->priv->ptr.has_nz_evo_analytic  = _FALSE_;
  cbe->priv->ptr.has_nz_evo_file      = _FALSE_;
  cbe->priv->ptr.bias                 = 0.0;
  cbe->priv->ptr.s_bias               = 0.0;

  /* spectra structure */

  cbe->priv->psp.z_max_pk = 0.0;
  cbe->priv->psp.non_diag = 0;

  /* lensing structure */

  cbe->priv->ple.has_lensed_cls = _FALSE_;

  /* nonlinear structure */

  cbe->priv->pnl.method = nl_none;

  /* all verbose parameters */

  cbe->priv->pba.background_verbose     = 0;
  cbe->priv->pth.thermodynamics_verbose = 0;
  cbe->priv->ppt.perturbations_verbose  = 0;
  cbe->priv->ptr.transfer_verbose       = 0;
  cbe->priv->ppm.primordial_verbose     = 0;
  cbe->priv->psp.spectra_verbose        = 0;
  cbe->priv->pnl.nonlinear_verbose      = 0;
  cbe->priv->ple.lensing_verbose        = 0;

  {
    guint verbosity = 0;
    cbe->bg_verbose       = verbosity;
    cbe->thermo_verbose   = verbosity;
    cbe->pert_verbose     = verbosity;
    cbe->transfer_verbose = verbosity;
    cbe->prim_verbose     = verbosity;
    cbe->spectra_verbose  = verbosity;
    cbe->nonlin_verbose   = verbosity;
    cbe->lensing_verbose  = verbosity;
  }
}

static void
_nc_cbe_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  NcCBE *cbe = NC_CBE (object);
  g_return_if_fail (NC_IS_CBE (object));

  switch (prop_id)
  {
    case PROP_PREC:
      nc_cbe_set_precision (cbe, g_value_get_object (value));
      break;
    case PROP_TARGET_CLS:
      nc_cbe_set_target_Cls (cbe, g_value_get_flags (value));
      break;
    case PROP_CALC_TRANSFER:
      nc_cbe_set_calc_transfer (cbe, g_value_get_boolean (value));
      break;
    case PROP_USE_LENSED_CLS:
      nc_cbe_set_lensed_Cls (cbe, g_value_get_boolean (value));
      break;
    case PROP_USE_TENSOR:
      nc_cbe_set_tensor (cbe, g_value_get_boolean (value));
      break;
    case PROP_USE_THERMODYN:
      nc_cbe_set_thermodyn (cbe, g_value_get_boolean (value));
      break;
    case PROP_SCALAR_LMAX:
      nc_cbe_set_scalar_lmax (cbe, g_value_get_uint (value));
      break;
    case PROP_VECTOR_LMAX:
      nc_cbe_set_vector_lmax (cbe, g_value_get_uint (value));
      break;
    case PROP_TENSOR_LMAX:
      nc_cbe_set_tensor_lmax (cbe, g_value_get_uint (value));
      break;
    case PROP_MATTER_PK_MAXZ:
      nc_cbe_set_max_matter_pk_z (cbe, g_value_get_double (value));
      break;
    case PROP_MATTER_PK_MAXK:
      nc_cbe_set_max_matter_pk_k (cbe, g_value_get_double (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
_nc_cbe_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  NcCBE *cbe = NC_CBE (object);
  g_return_if_fail (NC_IS_CBE (object));

  switch (prop_id)
  {
    case PROP_PREC:
      g_value_set_object (value, nc_cbe_peek_precision (cbe));
      break;
    case PROP_TARGET_CLS:
      g_value_set_flags (value, nc_cbe_get_target_Cls (cbe));
      break;
    case PROP_CALC_TRANSFER:
      g_value_set_boolean (value, nc_cbe_calc_transfer (cbe));
      break;
    case PROP_USE_LENSED_CLS:
      g_value_set_boolean (value, nc_cbe_lensed_Cls (cbe));
      break;
    case PROP_USE_TENSOR:
      g_value_set_boolean (value, nc_cbe_tensor (cbe));
      break;
    case PROP_USE_THERMODYN:
      g_value_set_boolean (value, nc_cbe_thermodyn (cbe));
      break;
    case PROP_SCALAR_LMAX:
      g_value_set_uint (value, nc_cbe_get_scalar_lmax (cbe));
      break;
    case PROP_VECTOR_LMAX:
      g_value_set_uint (value, nc_cbe_get_vector_lmax (cbe));
      break;
    case PROP_TENSOR_LMAX:
      g_value_set_uint (value, nc_cbe_get_tensor_lmax (cbe));
      break;
    case PROP_MATTER_PK_MAXZ:
      g_value_set_double (value, nc_cbe_get_max_matter_pk_z (cbe));
      break;
    case PROP_MATTER_PK_MAXK:
      g_value_set_double (value, nc_cbe_get_max_matter_pk_k (cbe));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
_nc_cbe_dispose (GObject *object)
{
  NcCBE *cbe = NC_CBE (object);

  nc_cbe_precision_clear (&cbe->prec);
  nc_scalefactor_clear (&cbe->a);
  ncm_model_ctrl_clear (&cbe->ctrl_cosmo);
  ncm_model_ctrl_clear (&cbe->ctrl_prim);

  /* Chain up : end */
  G_OBJECT_CLASS (nc_cbe_parent_class)->dispose (object);
}

static void _nc_cbe_free_thermo (NcCBE *cbe);

static void
_nc_cbe_finalize (GObject *object)
{
  NcCBE *cbe = NC_CBE (object);

  if (cbe->allocated)
  {
    g_assert (cbe->free != NULL);
    cbe->free (cbe);
    cbe->allocated = FALSE;
  }

  if (cbe->thermodyn_prepared)
  {
    _nc_cbe_free_thermo (cbe);
    cbe->thermodyn_prepared = FALSE;
  }
  
  /* Chain up : end */
  G_OBJECT_CLASS (nc_cbe_parent_class)->finalize (object);
}

static void
nc_cbe_class_init (NcCBEClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (NcCBEPrivate));

  object_class->set_property = &_nc_cbe_set_property;
  object_class->get_property = &_nc_cbe_get_property;
  object_class->dispose      = &_nc_cbe_dispose;
  object_class->finalize     = &_nc_cbe_finalize;

  g_object_class_install_property (object_class,
                                   PROP_PREC,
                                   g_param_spec_object ("precision",
                                                        NULL,
                                                        "CLASS precision object",
                                                        NC_TYPE_CBE_PRECISION,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_TARGET_CLS,
                                   g_param_spec_flags ("target-Cls",
                                                        NULL,
                                                        "Target Cls to calculate",
                                                        NC_TYPE_DATA_CMB_DATA_TYPE, 0,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_CALC_TRANSFER,
                                   g_param_spec_boolean ("calc-transfer",
                                                        NULL,
                                                        "Whether to calculate the transfer function",
                                                        FALSE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_USE_LENSED_CLS,
                                   g_param_spec_boolean ("use-lensed-Cls",
                                                        NULL,
                                                        "Whether to use lensed Cls",
                                                        FALSE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_USE_TENSOR,
                                   g_param_spec_boolean ("use-tensor",
                                                        NULL,
                                                        "Whether to use tensor contributions",
                                                        FALSE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_USE_THERMODYN,
                                   g_param_spec_boolean ("use-thermodyn",
                                                        NULL,
                                                        "Whether to use the thermodynamics module",
                                                        FALSE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_SCALAR_LMAX,
                                   g_param_spec_uint ("scalar-lmax",
                                                      NULL,
                                                      "Scalar modes l_max",
                                                      0, G_MAXUINT, 500,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_VECTOR_LMAX,
                                   g_param_spec_uint ("vector-lmax",
                                                      NULL,
                                                      "Vector modes l_max",
                                                      0, G_MAXUINT, 500,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));  
  g_object_class_install_property (object_class,
                                   PROP_TENSOR_LMAX,
                                   g_param_spec_uint ("tensor-lmax",
                                                      NULL,
                                                      "Tensor modes l_max",
                                                      0, G_MAXUINT, 500,
                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_MATTER_PK_MAXZ,
                                   g_param_spec_double ("matter-pk-maxz",
                                                        NULL,
                                                        "Maximum redshift for matter Pk",
                                                        0.0, G_MAXDOUBLE, 0.0,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_MATTER_PK_MAXK,
                                   g_param_spec_double ("matter-pk-maxk",
                                                        NULL,
                                                        "Maximum mode k for matter Pk",
                                                        0.0, G_MAXDOUBLE, 0.1,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB));
}

/**
 * nc_cbe_new: (constructor)
 *
 * FIXME
 *
 * Returns: (transfer full): a new #NcCBEPrecision.
 */
NcCBE *
nc_cbe_new (void)
{
  NcCBEPrecision *prec = nc_cbe_precision_new ();
  NcCBE *cbe = g_object_new (NC_TYPE_CBE,
                             "precision", prec,
                             NULL);
  nc_cbe_precision_free (prec);
  return cbe;
}

/**
 * nc_cbe_prec_new: (constructor)
 * @cbe_prec: a #NcCBEPrecision.
 *
 * FIXME
 *
 * Returns: (transfer full): a new #NcCBEPrecision.
 */
NcCBE *
nc_cbe_prec_new (NcCBEPrecision *cbe_prec)
{
  NcCBE *cbe = g_object_new (NC_TYPE_CBE,
                             "precision", cbe_prec,
                             NULL);
  return cbe;
}

/**
 * nc_cbe_ref:
 * @cbe: a #NcCBE
 *
 * Increases the reference count of @cbe.
 *
 * Returns: (transfer full): @cbe.
 */
NcCBE *
nc_cbe_ref (NcCBE *cbe)
{
  return g_object_ref (cbe);
}

/**
 * nc_cbe_free:
 * @cbe: a #NcCBE
 *
 * Decreases the reference count of @cbe.
 *
 */
void
nc_cbe_free (NcCBE *cbe)
{
  g_object_unref (cbe);
}

/**
 * nc_cbe_clear:
 * @cbe: a #NcCBE
 *
 * Decreases the reference count of *@cbe and sets *@cbe to NULL.
 *
 */
void
nc_cbe_clear (NcCBE **cbe)
{
  g_clear_object (cbe);
}

/**
 * nc_cbe_set_precision:
 * @cbe: a #NcCBE
 * @cbe_prec: a #NcCBEPrecision
 *
 * Sets the @cbe_prec as the precision object.
 *
 */
void 
nc_cbe_set_precision (NcCBE *cbe, NcCBEPrecision *cbe_prec)
{
  nc_cbe_precision_clear (&cbe->prec);
  cbe->prec = nc_cbe_precision_ref (cbe_prec);
  ncm_model_ctrl_force_update (cbe->ctrl_cosmo);
}

static void _nc_cbe_update_callbacks (NcCBE *cbe);

/**
 * nc_cbe_set_target_Cls:
 * @cbe: a #NcCBE
 * @target_Cls: a #NcDataCMBDataType.
 *
 * Sets the @target_Cls target.
 *
 */
void 
nc_cbe_set_target_Cls (NcCBE *cbe, NcDataCMBDataType target_Cls)
{
  if (cbe->target_Cls != target_Cls)
  {
    cbe->target_Cls = target_Cls;
    _nc_cbe_update_callbacks (cbe);
  }
}

/**
 * nc_cbe_set_calc_transfer:
 * @cbe: a #NcCBE
 * @calc_transfer: a boolean
 *
 * Sets whether it should calculate the transfer function.
 *
 */
void 
nc_cbe_set_calc_transfer (NcCBE *cbe, gboolean calc_transfer)
{
  if ((calc_transfer && !cbe->calc_transfer) || (!calc_transfer && cbe->calc_transfer))
  {
    cbe->calc_transfer = calc_transfer;
    _nc_cbe_update_callbacks (cbe);
  }
}

/**
 * nc_cbe_set_lensed_Cls:
 * @cbe: a #NcCBE
 * @use_lensed_Cls: a boolean.
 *
 * Sets whether it should use lensed Cl's.
 *
 */
void 
nc_cbe_set_lensed_Cls (NcCBE *cbe, gboolean use_lensed_Cls)
{
  cbe->use_lensed_Cls = use_lensed_Cls;
  _nc_cbe_update_callbacks (cbe);
}

/**
 * nc_cbe_set_tensor:
 * @cbe: a #NcCBE
 * @use_tensor: a boolean
 *
 * Sets whether it should use tensor contribution.
 *
 */
void 
nc_cbe_set_tensor (NcCBE *cbe, gboolean use_tensor)
{
  cbe->use_tensor = use_tensor;
  ncm_model_ctrl_force_update (cbe->ctrl_cosmo);
}

/**
 * nc_cbe_set_thermodyn:
 * @cbe: a #NcCBE
 * @use_thermodyn: a boolean
 *
 * Sets whether it should use the thermodynamics module.
 *
 */
void 
nc_cbe_set_thermodyn (NcCBE *cbe, gboolean use_thermodyn)
{
  cbe->use_thermodyn = use_thermodyn;
  ncm_model_ctrl_force_update (cbe->ctrl_cosmo);
}

/**
 * nc_cbe_set_scalar_lmax:
 * @cbe: a #NcCBE
 * @scalar_lmax: a guint
 *
 * Sets the maximum multipole $\ell_\textrm{max}$ at which the  
 * angular power spectrum $C_{\ell}$ of the scalar mode is computed.
 * 
 */
void 
nc_cbe_set_scalar_lmax (NcCBE *cbe, guint scalar_lmax)
{
  if (cbe->scalar_lmax != scalar_lmax)
  {
    cbe->scalar_lmax = scalar_lmax;
    ncm_model_ctrl_force_update (cbe->ctrl_cosmo);
  }
}

/**
 * nc_cbe_set_vector_lmax:
 * @cbe: a #NcCBE
 * @vector_lmax: a guint
 *
 * Sets the maximum multipole $\ell_\textrm{max}$ at which the  
 * angular power spectrum $C_{\ell}$ of the vector mode is computed.
 *
 */
void 
nc_cbe_set_vector_lmax (NcCBE *cbe, guint vector_lmax)
{
  if (cbe->vector_lmax != vector_lmax)
  {
    cbe->vector_lmax = vector_lmax;
    ncm_model_ctrl_force_update (cbe->ctrl_cosmo);
  }
}

/**
 * nc_cbe_set_tensor_lmax:
 * @cbe: a #NcCBE
 * @tensor_lmax: a guint
 *
 * Sets the maximum multipole $\ell_\textrm{max}$ at which the  
 * angular power spectrum $C_{\ell}$ of the tensor mode is computed.
 *
 */
void 
nc_cbe_set_tensor_lmax (NcCBE *cbe, guint tensor_lmax)
{
  if (cbe->tensor_lmax != tensor_lmax)
  {
    cbe->tensor_lmax = tensor_lmax;
    ncm_model_ctrl_force_update (cbe->ctrl_cosmo);
  }
}

/**
 * nc_cbe_set_max_matter_pk_z:
 * @cbe: a #NcCBE
 * @zmax: maximum redshift
 *
 * Sets $z_\mathrm{max}$ for (until?) which the matter power spectrum $P(k, z)$ is evaluated.
 *
 */
void 
nc_cbe_set_max_matter_pk_z (NcCBE *cbe, gdouble zmax)
{
  if (cbe->priv->psp.z_max_pk != zmax)
  {
    cbe->priv->psp.z_max_pk = zmax;
    ncm_model_ctrl_force_update (cbe->ctrl_cosmo);
  }
}

/**
 * nc_cbe_get_max_matter_pk_z:
 * @cbe: a #NcCBE
 *
 * Gets the maximum redshift $z_\mathrm{max}$ for which the matter power spectrum $P(k, z)$ is evaluated.
 * 
 * Returns: $z_\mathrm{max}$.
 */
gdouble
nc_cbe_get_max_matter_pk_z (NcCBE *cbe)
{
  return cbe->priv->psp.z_max_pk;
}

/**
 * nc_cbe_set_max_matter_pk_k:
 * @cbe: a #NcCBE
 * @kmax: maximum mode
 *
 * Sets $k_\mathrm{max}$ for which the matter power spectrum $P (k, z)$ is evaluated.
 *
 */
void 
nc_cbe_set_max_matter_pk_k (NcCBE *cbe, gdouble kmax)
{
  if (cbe->priv->ppt.k_max_for_pk != kmax)
  {
    cbe->priv->ppt.k_max_for_pk = kmax;
    ncm_model_ctrl_force_update (cbe->ctrl_cosmo);
  }
}

/**
 * nc_cbe_get_max_matter_pk_k:
 * @cbe: a #NcCBE
 *
 * Gets the maximum mode $k_\mathrm{max}$ for which the matter power spectrum $P (k, z)$ is evaluated.
 * 
 * Returns: $k_\mathrm{max}$.
 */
gdouble 
nc_cbe_get_max_matter_pk_k (NcCBE *cbe)
{
  return cbe->priv->ppt.k_max_for_pk;
}

/**
 * nc_cbe_peek_precision:
 * @cbe: a #NcCBE
 *
 * Peeks the #NcCBEPrecision object.
 * 
 * Returns: (transfer none): the #NcCBEPrecision object.
 */
NcCBEPrecision *
nc_cbe_peek_precision (NcCBE *cbe)
{
  return cbe->prec;
}

/**
 * nc_cbe_get_target_Cls:
 * @cbe: a #NcCBE
 *
 * Gets the target_Cls flags.
 * 
 * Returns: the #NcDataCMBDataType flags.
 */
NcDataCMBDataType 
nc_cbe_get_target_Cls (NcCBE *cbe)
{
  return cbe->target_Cls;
}

/**
 * nc_cbe_calc_transfer:
 * @cbe: a #NcCBE
 *
 * Gets whether it calculates the transfer function.
 * 
 * Returns: a boolean.
 */
gboolean 
nc_cbe_calc_transfer (NcCBE *cbe)
{
  return cbe->calc_transfer;
}

/**
 * nc_cbe_lensed_Cls:
 * @cbe: a #NcCBE
 *
 * Gets whether it uses lensed $C_{\ell}$'s.
 * 
 * Returns: a boolean.
 */
gboolean 
nc_cbe_lensed_Cls (NcCBE *cbe)
{
  return cbe->use_lensed_Cls;
}

/**
 * nc_cbe_tensor:
 * @cbe: a #NcCBE
 *
 * Gets whether it uses tensor contributions.
 * 
 * Returns: a boolean.
 */
gboolean 
nc_cbe_tensor (NcCBE *cbe)
{
  return cbe->use_tensor;
}

/**
 * nc_cbe_thermodyn:
 * @cbe: a #NcCBE
 *
 * Gets whether it uses the thermodynamics module.
 * 
 * Returns: a boolean.
 */
gboolean 
nc_cbe_thermodyn (NcCBE *cbe)
{
  return cbe->use_thermodyn;
}

/**
 * nc_cbe_get_scalar_lmax:
 * @cbe: a #NcCBE
 *
 * Gets the maximum multipole $\ell_\textrm{max}$ at which the  
 * angular power spectrum $C_{\ell}$ of the scalar mode is computed.
 * 
 * Returns: the maximum (scalar) multipole $\ell_\textrm{max}$.
 */
guint 
nc_cbe_get_scalar_lmax (NcCBE *cbe)
{
  return cbe->scalar_lmax;
}

/**
 * nc_cbe_get_vector_lmax:
 * @cbe: a #NcCBE
 *
 * Gets the maximum multipole $\ell_\textrm{max}$ at which the  
 * angular power spectrum $C_{\ell}$ of the vector mode is computed.
 * 
 * Returns: the maximum (vector) multipole $\ell_\textrm{max}$.
 */
guint 
nc_cbe_get_vector_lmax (NcCBE *cbe)
{
  return cbe->vector_lmax;
}

/**
 * nc_cbe_get_tensor_lmax:
 * @cbe: a #NcCBE
 *
 * Gets the maximum multipole $\ell_\textrm{max}$ at which the  
 * angular power spectrum $C_{\ell}$ of the tensor mode is computed.
 * 
 * Returns: the maximum (tensor) multipole $\ell_\textrm{max}$.
 */
guint 
nc_cbe_get_tensor_lmax (NcCBE *cbe)
{
  return cbe->tensor_lmax;
}

static void
_nc_cbe_set_bg (NcCBE *cbe, NcHICosmo *cosmo)
{
  if (!g_type_is_a (G_OBJECT_TYPE (cosmo), NC_TYPE_HICOSMO_DE))
    g_error ("_nc_cbe_set_bg: CLASS backend is compatible with darkenergy models only.");

  cbe->priv->pba.h                   = nc_hicosmo_h (cosmo);
  cbe->priv->pba.H0                  = cbe->priv->pba.h * 1.0e5 / ncm_c_c ();
  cbe->priv->pba.T_cmb               = nc_hicosmo_T_gamma0 (cosmo);
  cbe->priv->pba.Omega0_g            = nc_hicosmo_Omega_g0 (cosmo);
  cbe->priv->pba.Omega0_ur           = nc_hicosmo_Omega_nu0 (cosmo);
  cbe->priv->pba.Omega0_b            = nc_hicosmo_Omega_b0 (cosmo);
  cbe->priv->pba.Omega0_cdm          = nc_hicosmo_Omega_c0 (cosmo);
  cbe->priv->pba.Omega0_dcdmdr       = 0.0;
  cbe->priv->pba.Omega0_dcdm         = 0.0;
  cbe->priv->pba.Gamma_dcdm          = 0.0;
  cbe->priv->pba.N_ncdm              = 0;
  cbe->priv->pba.Omega0_ncdm_tot     = 0.;
  cbe->priv->pba.ksi_ncdm_default    = 0.;
  cbe->priv->pba.ksi_ncdm            = NULL;
  cbe->priv->pba.T_ncdm_default      = 0.71611;
  cbe->priv->pba.T_ncdm              = NULL;
  cbe->priv->pba.deg_ncdm_default    = 1.0;
  cbe->priv->pba.deg_ncdm            = NULL;
  cbe->priv->pba.ncdm_psd_parameters = NULL;
  cbe->priv->pba.ncdm_psd_files      = NULL;

  cbe->priv->pba.Omega0_scf          = 0.0;
  cbe->priv->pba.attractor_ic_scf    = _TRUE_;
  cbe->priv->pba.scf_parameters      = NULL;
  cbe->priv->pba.scf_parameters_size = 0;
  cbe->priv->pba.scf_tuning_index    = 0;
  cbe->priv->pba.phi_ini_scf         = 1;
  cbe->priv->pba.phi_prime_ini_scf   = 1;

  cbe->priv->pba.Omega0_k            = nc_hicosmo_Omega_k0 (cosmo);
  if (fabs (cbe->priv->pba.Omega0_k) > 1.0e-13)
  {
    cbe->priv->pba.a_today             = 1.0;
    cbe->priv->pba.K                   = - cbe->priv->pba.Omega0_k * gsl_pow_2 (cbe->priv->pba.a_today * cbe->priv->pba.H0);
    cbe->priv->pba.sgnK                = GSL_SIGN (cbe->priv->pba.K);
  }
  else
  {
    cbe->priv->pba.Omega0_k            = 0.0;
    cbe->priv->pba.K                   = 0.0;
    cbe->priv->pba.sgnK                = 0;
    cbe->priv->pba.a_today             = 1.0;
  }
  cbe->priv->pba.Omega0_lambda       = ncm_model_orig_param_get (NCM_MODEL (cosmo), NC_HICOSMO_DE_OMEGA_X);

  cbe->priv->pba.Omega0_fld          = 0.0;
  cbe->priv->pba.w0_fld              = -1.0;
  cbe->priv->pba.wa_fld              = 0.0;
  cbe->priv->pba.cs2_fld             = 1.0;

  cbe->priv->pba.background_verbose  = cbe->bg_verbose;
}

static void
_nc_cbe_set_thermo (NcCBE *cbe, NcHICosmo *cosmo)
{
  NcHIReion *reion = nc_hicosmo_peek_reion (cosmo);
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  g_assert (reion != NULL);

  cbe->priv->pth.YHe                      = nc_hicosmo_Yp_4He (cosmo);
  cbe->priv->pth.recombination            = recfast;
  cbe->priv->pth.reio_parametrization     = reio_camb;
  if (NC_IS_HIREION_CAMB (reion))
  {
    cbe->priv->pth.reio_z_or_tau            = reio_z;
    cbe->priv->pth.z_reio                   = ncm_model_orig_param_get (NCM_MODEL (reion), NC_HIREION_CAMB_HII_HEII_Z);
    cbe->priv->pth.tau_reio                 = nc_hireion_get_tau (reion, cosmo);
  }
  else
  {
    cbe->priv->pth.reio_z_or_tau            = reio_tau;
    cbe->priv->pth.z_reio                   = 13.0;
    cbe->priv->pth.tau_reio                 = nc_hireion_get_tau (reion, cosmo);
  }
  cbe->priv->pth.reionization_exponent    = 1.5;
  cbe->priv->pth.reionization_width       = 0.5;
  cbe->priv->pth.helium_fullreio_redshift = 3.5;
  cbe->priv->pth.helium_fullreio_width    = 0.5;
  cbe->priv->pth.binned_reio_num            = 0;
  cbe->priv->pth.binned_reio_z              = NULL;
  cbe->priv->pth.binned_reio_xe             = NULL;
  cbe->priv->pth.binned_reio_step_sharpness = 0.3;

  cbe->priv->pth.annihilation           = 0.0;
  cbe->priv->pth.decay                  = 0.0;
  cbe->priv->pth.annihilation_variation = 0.0;
  cbe->priv->pth.annihilation_z         = 1000.0;
  cbe->priv->pth.annihilation_zmax      = 2500.0;
  cbe->priv->pth.annihilation_zmin      = 30.0;
  cbe->priv->pth.annihilation_f_halo    = 0.0;
  cbe->priv->pth.annihilation_z_halo    = 30.0;
  cbe->priv->pth.has_on_the_spot        = _TRUE_;

  cbe->priv->pth.compute_cb2_derivatives = _FALSE_;

  cbe->priv->pth.thermodynamics_verbose  = cbe->thermo_verbose;

  if ((ppr->tight_coupling_approximation == (gint) first_order_CLASS) ||
      (ppr->tight_coupling_approximation == (gint) second_order_CLASS))
    cbe->priv->pth.compute_cb2_derivatives = _TRUE_;
}

static void
_nc_cbe_set_pert (NcCBE *cbe, NcHICosmo *cosmo)
{
  gboolean has_cls = (cbe->target_Cls & NC_DATA_CMB_TYPE_ALL) != 0;
  gboolean has_perturbations = has_cls || cbe->calc_transfer;
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  /*
   * Inside CLASS they compare booleans with _TRUE_ and _FALSE_.
   * This is a bad idea, but to be compatible we must always
   * use their _TRUE_ and _FALSE_.
   */

  if (cbe->target_Cls & (NC_DATA_CMB_TYPE_TB | NC_DATA_CMB_TYPE_EB))
    g_error ("_nc_cbe_set_pert: modes TB and EB are not supported.");

  cbe->priv->ppt.has_perturbations            = has_perturbations ? _TRUE_ : _FALSE_;
  cbe->priv->ppt.has_cls                      = has_cls ? _TRUE_ : _FALSE_;

  cbe->priv->ppt.has_cl_cmb_temperature       = cbe->target_Cls & NC_DATA_CMB_TYPE_TT ? _TRUE_ : _FALSE_;
  cbe->priv->ppt.has_cl_cmb_polarization      =
    cbe->target_Cls & (NC_DATA_CMB_TYPE_EE | NC_DATA_CMB_TYPE_BB | NC_DATA_CMB_TYPE_TE) ? _TRUE_ : _FALSE_;
  cbe->priv->ppt.has_cl_cmb_lensing_potential = _TRUE_;
  cbe->priv->ppt.has_cl_number_count          = _FALSE_;
  cbe->priv->ppt.has_cl_lensing_potential     = _FALSE_;
  cbe->priv->ppt.has_pk_matter                = cbe->calc_transfer ? _TRUE_ : _FALSE_;
  cbe->priv->ppt.has_density_transfers        = _FALSE_;
  cbe->priv->ppt.has_velocity_transfers       = _FALSE_;

  cbe->priv->ppt.has_nl_corrections_based_on_delta_m = _FALSE_;

  cbe->priv->ppt.has_nc_density = _FALSE_;
  cbe->priv->ppt.has_nc_rsd     = _FALSE_;
  cbe->priv->ppt.has_nc_lens    = _FALSE_;
  cbe->priv->ppt.has_nc_gr      = _FALSE_;

  cbe->priv->ppt.switch_sw         = 1;
  cbe->priv->ppt.switch_eisw       = 1;
  cbe->priv->ppt.switch_lisw       = 1;
  cbe->priv->ppt.switch_dop        = 1;
  cbe->priv->ppt.switch_pol        = 1;
  cbe->priv->ppt.eisw_lisw_split_z = 120;

  cbe->priv->ppt.has_ad  = _TRUE_;
  cbe->priv->ppt.has_bi  = _FALSE_;
  cbe->priv->ppt.has_cdi = _FALSE_;
  cbe->priv->ppt.has_nid = _FALSE_;
  cbe->priv->ppt.has_niv = _FALSE_;

  cbe->priv->ppt.has_perturbed_recombination = _FALSE_;
  cbe->priv->ppt.tensor_method               = tm_massless_approximation;
  cbe->priv->ppt.evolve_tensor_ur            = _FALSE_;
  cbe->priv->ppt.evolve_tensor_ncdm          = _FALSE_;

  cbe->priv->ppt.has_scalars = _TRUE_;
  cbe->priv->ppt.has_vectors = _FALSE_;
  cbe->priv->ppt.has_tensors = cbe->use_tensor ? _TRUE_ : _FALSE_;

  cbe->priv->ppt.l_scalar_max = cbe->scalar_lmax +
    (cbe->use_lensed_Cls ? ppr->delta_l_max : 0);

  cbe->priv->ppt.l_vector_max = cbe->vector_lmax;
  cbe->priv->ppt.l_tensor_max = cbe->tensor_lmax;
  cbe->priv->ppt.l_lss_max    = 300;
  /*cbe->priv->ppt.k_max_for_pk = 0.1;*/

  cbe->priv->ppt.gauge = synchronous;

  cbe->priv->ppt.k_output_values_num     = 0;
  cbe->priv->ppt.store_perturbations     = _FALSE_;
  cbe->priv->ppt.number_of_scalar_titles = 0;
  cbe->priv->ppt.number_of_vector_titles = 0;
  cbe->priv->ppt.number_of_tensor_titles = 0;
  {
    guint filenum;
    for (filenum = 0; filenum<_MAX_NUMBER_OF_K_FILES_; filenum++)
    {
      cbe->priv->ppt.scalar_perturbations_data[filenum] = NULL;
      cbe->priv->ppt.vector_perturbations_data[filenum] = NULL;
      cbe->priv->ppt.tensor_perturbations_data[filenum] = NULL;
    }
  }
  cbe->priv->ppt.index_k_output_values = NULL;

  cbe->priv->ppt.selection_num      = 1;
  cbe->priv->ppt.selection          = gaussian;
  cbe->priv->ppt.selection_mean[0]  = 1.0;
  cbe->priv->ppt.selection_width[0] = 0.1;

  cbe->priv->ppt.perturbations_verbose = cbe->pert_verbose;
}

static gdouble
_external_Pk_callback_pks (const double lnk, gpointer data)
{
  NcHIPrim *prim = NC_HIPRIM (data);

  return nc_hiprim_lnSA_powspec_lnk (prim, lnk);
}

static gdouble
_external_Pk_callback_pkt (const double lnk, gpointer data)
{
  NcHIPrim *prim = NC_HIPRIM (data);

  return nc_hiprim_lnT_powspec_lnk (prim, lnk);
}

static void
_nc_cbe_set_prim (NcCBE *cbe, NcHICosmo *cosmo)
{
  NcHIPrim *prim = nc_hicosmo_peek_prim (cosmo);
  
  cbe->priv->ppm.primordial_spec_type = external_Pk_callback;
  /*cbe->priv->ppm.primordial_spec_type = analytic_Pk;*/
  cbe->priv->ppm.external_Pk_callback_pks  = &_external_Pk_callback_pks;
  if (cbe->use_tensor)
  {
    g_assert (ncm_model_impl (NCM_MODEL (prim)) & NC_HIPRIM_IMPL_lnT_powspec_lnk);
    cbe->priv->ppm.external_Pk_callback_pkt  = &_external_Pk_callback_pkt;
  }
  cbe->priv->ppm.external_Pk_callback_data = prim;

  cbe->priv->ppm.k_pivot       = 0.05;
  cbe->priv->ppm.A_s           = 2.40227188179e-9;
  cbe->priv->ppm.n_s           = 0.9742;
  cbe->priv->ppm.alpha_s       = 0.0;
  cbe->priv->ppm.f_bi          = 1.0;
  cbe->priv->ppm.n_bi          = 1.0;
  cbe->priv->ppm.alpha_bi      = 0.0;
  cbe->priv->ppm.f_cdi         = 1.0;
  cbe->priv->ppm.n_cdi         = 1.0;
  cbe->priv->ppm.alpha_cdi     = 0.0;
  cbe->priv->ppm.f_nid         = 1.0;
  cbe->priv->ppm.n_nid         = 1.0;
  cbe->priv->ppm.alpha_nid     = 0.0;
  cbe->priv->ppm.f_niv         = 1.0;
  cbe->priv->ppm.n_niv         = 1.0;
  cbe->priv->ppm.alpha_niv     = 0.0;
  cbe->priv->ppm.c_ad_bi       = 0.0;
  cbe->priv->ppm.n_ad_bi       = 0.0;
  cbe->priv->ppm.alpha_ad_bi   = 0.0;
  cbe->priv->ppm.c_ad_cdi      = 0.0;
  cbe->priv->ppm.n_ad_cdi      = 0.0;
  cbe->priv->ppm.alpha_ad_cdi  = 0.0;
  cbe->priv->ppm.c_ad_nid      = 0.0;
  cbe->priv->ppm.n_ad_nid      = 0.0;
  cbe->priv->ppm.alpha_ad_nid  = 0.0;
  cbe->priv->ppm.c_ad_niv      = 0.0;
  cbe->priv->ppm.n_ad_niv      = 0.0;
  cbe->priv->ppm.alpha_ad_niv  = 0.0;
  cbe->priv->ppm.c_bi_cdi      = 0.0;
  cbe->priv->ppm.n_bi_cdi      = 0.0;
  cbe->priv->ppm.alpha_bi_cdi  = 0.0;
  cbe->priv->ppm.c_bi_nid      = 0.0;
  cbe->priv->ppm.n_bi_nid      = 0.0;
  cbe->priv->ppm.alpha_bi_nid  = 0.0;
  cbe->priv->ppm.c_bi_niv      = 0.0;
  cbe->priv->ppm.n_bi_niv      = 0.0;
  cbe->priv->ppm.alpha_bi_niv  = 0.0;
  cbe->priv->ppm.c_cdi_nid     = 0.0;
  cbe->priv->ppm.n_cdi_nid     = 0.0;
  cbe->priv->ppm.alpha_cdi_nid = 0.0;
  cbe->priv->ppm.c_cdi_niv     = 0.0;
  cbe->priv->ppm.n_cdi_niv     = 0.0;
  cbe->priv->ppm.alpha_cdi_niv = 0.0;
  cbe->priv->ppm.c_nid_niv     = 0.0;
  cbe->priv->ppm.n_nid_niv     = 0.0;
  cbe->priv->ppm.alpha_nid_niv = 0.0;
  cbe->priv->ppm.r           = 1.0;
  cbe->priv->ppm.n_t         = -cbe->priv->ppm.r / 8.0 * (2.0 - cbe->priv->ppm.r / 8.0 - cbe->priv->ppm.n_s);
  cbe->priv->ppm.alpha_t     = cbe->priv->ppm.r / 8.0 * (cbe->priv->ppm.r / 8.0 + cbe->priv->ppm.n_s - 1.0);
  cbe->priv->ppm.potential   = polynomial;
  cbe->priv->ppm.phi_end     = 0.0;
  cbe->priv->ppm.ln_aH_ratio = 50;
  cbe->priv->ppm.V0          = 1.25e-13;
  cbe->priv->ppm.V1          = -1.12e-14;
  cbe->priv->ppm.V2          = -6.95e-14;
  cbe->priv->ppm.V3          = 0.0;
  cbe->priv->ppm.V4          = 0.0;
  cbe->priv->ppm.H0          = 3.69e-6;
  cbe->priv->ppm.H1          = -5.84e-7;
  cbe->priv->ppm.H2          = 0.0;
  cbe->priv->ppm.H3          = 0.0;
  cbe->priv->ppm.H4          = 0.0;
  cbe->priv->ppm.command     = "write here your command for the external Pk";
  cbe->priv->ppm.custom1     = 0.0;
  cbe->priv->ppm.custom2     = 0.0;
  cbe->priv->ppm.custom3     = 0.0;
  cbe->priv->ppm.custom4     = 0.0;
  cbe->priv->ppm.custom5     = 0.0;
  cbe->priv->ppm.custom6     = 0.0;
  cbe->priv->ppm.custom7     = 0.0;
  cbe->priv->ppm.custom8     = 0.0;
  cbe->priv->ppm.custom9     = 0.0;
  cbe->priv->ppm.custom10    = 0.0;

  cbe->priv->ppm.primordial_verbose = cbe->prim_verbose;
}

static void
_nc_cbe_set_transfer (NcCBE *cbe, NcHICosmo *cosmo)
{
  cbe->priv->ptr.lcmb_rescale         = 1.0;
  cbe->priv->ptr.lcmb_pivot           = 0.1;
  cbe->priv->ptr.lcmb_tilt            = 0.0;
  cbe->priv->ptr.initialise_HIS_cache = _FALSE_;
  cbe->priv->ptr.has_nz_analytic      = _FALSE_;
  cbe->priv->ptr.has_nz_file          = _FALSE_;
  cbe->priv->ptr.has_nz_evo_analytic  = _FALSE_;
  cbe->priv->ptr.has_nz_evo_file      = _FALSE_;
  cbe->priv->ptr.bias                 = 1.0;
  cbe->priv->ptr.s_bias               = 0.0;

  cbe->priv->ptr.transfer_verbose = cbe->transfer_verbose;
}

static void
_nc_cbe_set_spectra (NcCBE *cbe, NcHICosmo *cosmo)
{
  /*cbe->priv->psp.z_max_pk = 0.0;*/
  cbe->priv->psp.non_diag = 0;

  cbe->priv->psp.spectra_verbose = cbe->spectra_verbose;
}

static void
_nc_cbe_set_lensing (NcCBE *cbe, NcHICosmo *cosmo)
{
  cbe->priv->ple.has_lensed_cls  = cbe->use_lensed_Cls ? _TRUE_ : _FALSE_;
  cbe->priv->ple.lensing_verbose = cbe->lensing_verbose;
}

static void
_nc_cbe_set_nonlin (NcCBE *cbe, NcHICosmo *cosmo)
{
  cbe->priv->pnl.method = nl_none;
  cbe->priv->pnl.nonlinear_verbose = cbe->nonlin_verbose;
}

static void _nc_cbe_free_bg (NcCBE *cbe);
static void _nc_cbe_free_thermo (NcCBE *cbe);
static void _nc_cbe_free_pert (NcCBE *cbe);
static void _nc_cbe_free_prim (NcCBE *cbe);
static void _nc_cbe_free_nonlin (NcCBE *cbe);
static void _nc_cbe_free_transfer (NcCBE *cbe);
static void _nc_cbe_free_spectra (NcCBE *cbe);
static void _nc_cbe_free_lensing (NcCBE *cbe);

static void
_nc_cbe_call_bg (NcCBE *cbe, NcHICosmo *cosmo)
{
  struct precision *ppr = (struct precision *)cbe->prec->priv;
  
  _nc_cbe_set_bg (cbe, cosmo);
  if (background_init (ppr, &cbe->priv->pba) == _FAILURE_)
    g_error ("_nc_cbe_call_bg: Error running background_init `%s'\n", cbe->priv->pba.error_message);
  
  if (FALSE)
  {
    const gdouble RH = nc_hicosmo_RH_Mpc (cosmo);
    gdouble zf = 1.0 / ppr->a_ini_over_a_today_default;
    struct background *pba = &cbe->priv->pba;
    gdouble pvecback[pba->bg_size];
    gdouble err = 0.0;

    if (cbe->a == NULL)
      cbe->a = nc_scalefactor_new (NC_SCALEFACTOR_TIME_TYPE_COSMIC, zf, NULL);
    else
      nc_scalefactor_set_zf (cbe->a, zf);

    nc_scalefactor_prepare_if_needed (cbe->a, cosmo);

    guint i;

    for (i = 0; i < pba->bt_size; i++)
    {
      gint last_index = 0;
      const gdouble eta       = pba->tau_table[i] / RH;
      const gdouble z         = nc_scalefactor_z_eta (cbe->a, eta);
      const gdouble x         = 1.0 + z;
      const gdouble x2        = x * x;
      const gdouble x3        = x2 * x;
      const gdouble x4        = x2 * x2;
      const gdouble E2        = nc_hicosmo_E2 (cosmo, z);
      const gdouble E         = sqrt (E2);
      const gdouble H         = E / RH;
      const gdouble RH_pow_m2 = 1.0 / (RH * RH);
      const gdouble H_prime   = - 0.5 * nc_hicosmo_dE2_dz (cosmo, z) * RH_pow_m2;

      const gdouble rho_g      = nc_hicosmo_Omega_g0 (cosmo) * RH_pow_m2 * x4;
      const gdouble rho_ur     = nc_hicosmo_Omega_nu0 (cosmo) * RH_pow_m2 * x4;
      const gdouble rho_b      = nc_hicosmo_Omega_b0 (cosmo) * RH_pow_m2 * x3;
      const gdouble rho_cdm    = nc_hicosmo_Omega_c0 (cosmo) * RH_pow_m2 * x3;
      const gdouble rho_Lambda = ncm_model_orig_param_get (NCM_MODEL (cosmo), NC_HICOSMO_DE_OMEGA_X) * RH_pow_m2;

      const gdouble E2Omega_t  = nc_hicosmo_E2Omega_t (cosmo, z);
      const gdouble Omega_r    = nc_hicosmo_Omega_r0 (cosmo) * x4 / E2Omega_t;
      const gdouble Omega_m    = nc_hicosmo_Omega_m0 (cosmo) * x3 / E2Omega_t;

      const gdouble rho_crit   = E2 * RH_pow_m2;

      background_at_tau (pba,
                         pba->tau_table[i],
                         pba->long_info,
                         pba->inter_normal,
                         &last_index,
                         pvecback);

      {
        
        const gdouble a_diff      = fabs (nc_scalefactor_a_eta (cbe->a, eta) / pvecback[pba->index_bg_a] - 1.0);
        const gdouble H_diff      = fabs (H / pvecback[pba->index_bg_H] - 1.0);
        const gdouble Hprime_diff = fabs (H_prime / pvecback[pba->index_bg_H_prime] - 1.0);

        const gdouble rho_g_diff      = fabs (rho_g / pvecback[pba->index_bg_rho_g] - 1.0);
        const gdouble rho_ur_diff     = fabs (rho_ur / pvecback[pba->index_bg_rho_ur] - 1.0);
        const gdouble rho_b_diff      = fabs (rho_b / pvecback[pba->index_bg_rho_b] - 1.0);
        const gdouble rho_cdm_diff    = fabs (rho_cdm / pvecback[pba->index_bg_rho_cdm] - 1.0);
        const gdouble rho_Lambda_diff = fabs (rho_Lambda / pvecback[pba->index_bg_rho_lambda] - 1.0);

        const gdouble Omega_m0_diff   = fabs (Omega_m / pvecback[pba->index_bg_Omega_m] - 1.0);
        const gdouble Omega_r0_diff   = fabs (Omega_r / pvecback[pba->index_bg_Omega_r] - 1.0);

        const gdouble rho_crit_diff   = fabs (rho_crit / pvecback[pba->index_bg_rho_crit] - 1.0);

        err = GSL_MAX (err, a_diff);
        err = GSL_MAX (err, H_diff);
        err = GSL_MAX (err, Hprime_diff);
        err = GSL_MAX (err, rho_g_diff);
        err = GSL_MAX (err, rho_ur_diff);
        err = GSL_MAX (err, rho_b_diff);
        err = GSL_MAX (err, rho_cdm_diff);
        err = GSL_MAX (err, rho_Lambda_diff);
        err = GSL_MAX (err, Omega_r0_diff);
        err = GSL_MAX (err, rho_crit_diff);
        
        printf ("# eta = % 20.15g | % 10.5e % 10.5e % 10.5e % 10.5e % 10.5e % 10.5e % 10.5e % 10.5e % 10.5e % 10.5e % 10.5e % 10.5e\n", eta,
                pvecback[pba->index_bg_a], a_diff, H_diff, Hprime_diff, rho_g_diff, rho_ur_diff, rho_b_diff, rho_cdm_diff, rho_Lambda_diff,
                Omega_r0_diff, Omega_m0_diff, rho_crit_diff
                );

      }
    }
    printf ("# worst % 10.5e\n", err);
  }  
}

static void
_nc_cbe_call_thermo (NcCBE *cbe, NcHICosmo *cosmo)
{
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  _nc_cbe_call_bg (cbe, cosmo);
  
  _nc_cbe_set_thermo (cbe, cosmo);
  if (thermodynamics_init (ppr, &cbe->priv->pba, &cbe->priv->pth) == _FAILURE_)
    g_error ("_nc_cbe_call_thermo: Error running thermodynamics_init `%s'\n", cbe->priv->pth.error_message);
}

static void
_nc_cbe_call_pert (NcCBE *cbe, NcHICosmo *cosmo)
{
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  /*_nc_cbe_call_thermo (cbe, cosmo);*/
  cbe->free = &_nc_cbe_free_pert;
  
  _nc_cbe_set_pert (cbe, cosmo);
  if (perturb_init (ppr, &cbe->priv->pba, &cbe->priv->pth, &cbe->priv->ppt) == _FAILURE_)
    g_error ("_nc_cbe_call_pert: Error running perturb_init `%s'\n", cbe->priv->ppt.error_message);
}

static void
_nc_cbe_call_prim (NcCBE *cbe, NcHICosmo *cosmo)
{
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  _nc_cbe_call_pert (cbe, cosmo);
  cbe->free = &_nc_cbe_free_prim;

  _nc_cbe_set_prim (cbe, cosmo);
  if (primordial_init (ppr, &cbe->priv->ppt, &cbe->priv->ppm) == _FAILURE_)
    g_error ("_nc_cbe_call_prim: Error running primordial_init `%s'\n", cbe->priv->ppm.error_message);  
}

static void
_nc_cbe_call_nonlin (NcCBE *cbe, NcHICosmo *cosmo)
{
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  _nc_cbe_call_prim (cbe, cosmo);
  cbe->free = &_nc_cbe_free_nonlin;

  _nc_cbe_set_nonlin (cbe, cosmo);
  if (nonlinear_init (ppr, &cbe->priv->pba, &cbe->priv->pth, &cbe->priv->ppt, &cbe->priv->ppm, &cbe->priv->pnl) == _FAILURE_)
    g_error ("_nc_cbe_call_nonlin: Error running nonlinear_init `%s'\n", cbe->priv->pnl.error_message);
}

static void
_nc_cbe_call_transfer (NcCBE *cbe, NcHICosmo *cosmo)
{
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  _nc_cbe_call_nonlin (cbe, cosmo);
  cbe->free = &_nc_cbe_free_transfer;

  _nc_cbe_set_transfer (cbe, cosmo);
  if (transfer_init (ppr, &cbe->priv->pba, &cbe->priv->pth, &cbe->priv->ppt, &cbe->priv->pnl, &cbe->priv->ptr) == _FAILURE_)
    g_error ("_nc_cbe_call_transfer: Error running transfer_init `%s'\n", cbe->priv->ptr.error_message);
}

static void
_nc_cbe_call_spectra (NcCBE *cbe, NcHICosmo *cosmo)
{
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  _nc_cbe_call_transfer (cbe, cosmo);
  cbe->free = &_nc_cbe_free_spectra;

  _nc_cbe_set_spectra (cbe, cosmo);
  if (spectra_init (ppr, &cbe->priv->pba, &cbe->priv->ppt, &cbe->priv->ppm, &cbe->priv->pnl, &cbe->priv->ptr, &cbe->priv->psp) == _FAILURE_)
    g_error ("_nc_cbe_call_spectra: Error running spectra_init `%s'\n", cbe->priv->psp.error_message);  
}

static void
_nc_cbe_call_lensing (NcCBE *cbe, NcHICosmo *cosmo)
{
  struct precision *ppr = (struct precision *)cbe->prec->priv;

  _nc_cbe_call_spectra (cbe, cosmo);
  cbe->free = &_nc_cbe_free_lensing;

  _nc_cbe_set_lensing (cbe, cosmo);
  if (lensing_init (ppr, &cbe->priv->ppt, &cbe->priv->psp, &cbe->priv->pnl, &cbe->priv->ple) == _FAILURE_)
    g_error ("_nc_cbe_call_lensing: Error running lensing_init `%s'\n", cbe->priv->ple.error_message);
}

static void
_nc_cbe_free_bg (NcCBE *cbe)
{
  if (background_free (&cbe->priv->pba) == _FAILURE_)
    g_error ("_nc_cbe_free_bg: Error running background_free `%s'\n", cbe->priv->pba.error_message);  
}

static void
_nc_cbe_free_thermo (NcCBE *cbe)
{
  if (thermodynamics_free (&cbe->priv->pth) == _FAILURE_)
    g_error ("_nc_cbe_free_thermo: Error running thermodynamics_free `%s'\n", cbe->priv->pth.error_message);

  _nc_cbe_free_bg (cbe);
}

static void
_nc_cbe_free_pert (NcCBE *cbe)
{
  if (perturb_free (&cbe->priv->ppt) == _FAILURE_)
    g_error ("_nc_cbe_free_pert: Error running perturb_free `%s'\n", cbe->priv->ppt.error_message);

  /*_nc_cbe_free_thermo (cbe);*/
}

static void
_nc_cbe_free_prim (NcCBE *cbe)
{
  if (primordial_free (&cbe->priv->ppm) == _FAILURE_)
    g_error ("_nc_cbe_free_prim: Error running primordial_free `%s'\n", cbe->priv->ppm.error_message);

  _nc_cbe_free_pert (cbe);
}

static void
_nc_cbe_free_nonlin (NcCBE *cbe)
{
  if (nonlinear_free (&cbe->priv->pnl) == _FAILURE_)
    g_error ("_nc_cbe_free_nonlin: Error running nonlinear_free `%s'\n", cbe->priv->pnl.error_message);

  _nc_cbe_free_prim (cbe);
}

static void
_nc_cbe_free_transfer (NcCBE *cbe)
{
  if (transfer_free (&cbe->priv->ptr) == _FAILURE_)
    g_error ("_nc_cbe_free_transfer: Error running transfer_free `%s'\n", cbe->priv->ptr.error_message);

  _nc_cbe_free_nonlin (cbe);
}

static void
_nc_cbe_free_spectra (NcCBE *cbe)
{
  if (spectra_free (&cbe->priv->psp) == _FAILURE_)
    g_error ("_nc_cbe_free_lensing: Error running spectra_free `%s'\n", cbe->priv->psp.error_message);

  _nc_cbe_free_transfer (cbe);
}

static void
_nc_cbe_free_lensing (NcCBE *cbe)
{
  if (lensing_free (&cbe->priv->ple) == _FAILURE_)
    g_error ("_nc_cbe_free_lensing: Error running lensing_free `%s'\n", cbe->priv->ple.error_message);

  _nc_cbe_free_spectra (cbe);
}

static void
_nc_cbe_update_callbacks (NcCBE *cbe)
{
  gboolean has_Cls = cbe->target_Cls & NC_DATA_CMB_TYPE_ALL;
  
  ncm_model_ctrl_force_update (cbe->ctrl_cosmo);

  if (cbe->allocated)
  {
    g_assert (cbe->free != NULL);
    cbe->free (cbe);
    cbe->call      = NULL;
    cbe->free      = NULL;
    cbe->allocated = FALSE;
  }
  
  if (has_Cls && cbe->use_lensed_Cls)
    cbe->call = _nc_cbe_call_lensing;
  else if (has_Cls || cbe->calc_transfer)
    cbe->call = _nc_cbe_call_spectra;
}

/**
 * nc_cbe_thermodyn_prepare:
 * @cbe: a #NcCBE
 * @cosmo: a #NcHICosmo
 * 
 * Prepares the thermodynamic Class structure.
 * 
 */
void
nc_cbe_thermodyn_prepare (NcCBE *cbe, NcHICosmo *cosmo)
{
  if (cbe->thermodyn_prepared)
  {
    _nc_cbe_free_thermo (cbe);
    cbe->thermodyn_prepared = FALSE;
  }

  _nc_cbe_call_thermo (cbe, cosmo);
  cbe->thermodyn_prepared = TRUE;
}

/**
 * nc_cbe_thermodyn_prepare_if_needed:
 * @cbe: a #NcCBE
 * @cosmo: a #NcHICosmo
 * 
 * Prepares the thermodynamic Class structure.
 * 
 */
void
nc_cbe_thermodyn_prepare_if_needed (NcCBE *cbe, NcHICosmo *cosmo)
{
  if (ncm_model_ctrl_update (cbe->ctrl_cosmo, NCM_MODEL (cosmo)))
  {
    nc_cbe_thermodyn_prepare (cbe, cosmo);
    ncm_model_ctrl_force_update (cbe->ctrl_prim);
  }
}

/**
 * nc_cbe_prepare:
 * @cbe: a #NcCBE
 * @cosmo: a #NcHICosmo
 * 
 * Prepares all necessary Class structures.
 * 
 */
void
nc_cbe_prepare (NcCBE *cbe, NcHICosmo *cosmo)
{
  /*printf ("Preparing CLASS!\n");*/
  if (cbe->allocated)
  {
    g_assert (cbe->free != NULL);
    cbe->free (cbe);
    cbe->allocated = FALSE;
  }

  if (cbe->thermodyn_prepared)
  {
    _nc_cbe_free_thermo (cbe);
    cbe->thermodyn_prepared = FALSE;
  }

  _nc_cbe_call_thermo (cbe, cosmo);
  cbe->thermodyn_prepared = TRUE;

  if (cbe->call != NULL)
  {
    cbe->call (cbe, cosmo);
    cbe->allocated = TRUE;
  }
}

/**
 * nc_cbe_prepare_if_needed:
 * @cbe: a #NcCBE
 * @cosmo: a #NcHICosmo
 * 
 * Prepares all necessary Class structures.
 * 
 */
void
nc_cbe_prepare_if_needed (NcCBE *cbe, NcHICosmo *cosmo)
{
  ncm_model_ctrl_update (cbe->ctrl_cosmo, NCM_MODEL (cosmo));

  if (!ncm_model_ctrl_model_has_submodel (cbe->ctrl_cosmo, nc_hiprim_id ()))
  {
    g_error ("nc_cbe_prepare_if_needed: cosmo model must contain a NcHIPrim submodel.");
  }
  else
  {
    gboolean cosmo_up = ncm_model_ctrl_model_last_update (cbe->ctrl_cosmo);
    gboolean prim_up  = ncm_model_ctrl_submodel_last_update (cbe->ctrl_cosmo, nc_hiprim_id ());

    /*printf ("cosmo_up %d prim_up %d [%p]\n", cosmo_up, prim_up, cosmo);*/
    
    if (cosmo_up)
    {    
      nc_cbe_prepare (cbe, cosmo);
    }
    else if (prim_up)
    {
      if (cbe->allocated)
      {
        g_assert (cbe->free != NULL);
        cbe->free (cbe);
        cbe->allocated = FALSE;
      }
      if (cbe->call != NULL)
      {
        cbe->call (cbe, cosmo);
        cbe->allocated = TRUE;
      }
    }
  }
}

/**
 * nc_cbe_thermodyn_get_Xe:
 * @cbe: a #NcCBE
 * 
 * Gets the free electrons fraction $X_e$ as a function of the redshift.
 * 
 * Returns: (transfer full): a #NcmSpline for Xe.
 */
NcmSpline *
nc_cbe_thermodyn_get_Xe (NcCBE *cbe)
{
  const guint size = cbe->priv->pth.tt_size;
  NcmVector *z_v  = ncm_vector_new (size);
  NcmVector *Xe_v = ncm_vector_new (size);
  NcmSpline *Xe_s = ncm_spline_cubic_notaknot_new_full (z_v, Xe_v, FALSE);
  guint i;

  for (i = 0; i < size; i++)
  {
    const gdouble z_i  = cbe->priv->pth.z_table[i];
    const gdouble Xe_i = cbe->priv->pth.thermodynamics_table[cbe->priv->pth.th_size * i + cbe->priv->pth.index_th_xe];
    
    ncm_vector_fast_set (z_v,  size - 1 - i, -log (z_i + 1.0));
    ncm_vector_fast_set (Xe_v, size - 1 - i, Xe_i);
  }
  
  ncm_vector_clear (&z_v);
  ncm_vector_clear (&Xe_v);

  ncm_spline_prepare (Xe_s);

  return Xe_s;
}

/**
 * nc_cbe_get_matter_ps:
 * @cbe: a #NcCBE
 * 
 * Gets the logarithm base e of the matter power spectrum as a function of the redshift $z$ and mode $\ln (k)$.
 * 
 * Returns: (transfer full): a #NcmSpline2d for the logarithm base e of the matter power spectrum, $\ln P(\ln k, z)$.
 */
NcmSpline2d *
nc_cbe_get_matter_ps (NcCBE *cbe)
{
  NcmVector *lnk_v = ncm_vector_new (cbe->priv->psp.ln_k_size);
  NcmVector *z_v   = ncm_vector_new (cbe->priv->psp.ln_tau_size);
  NcmMatrix *lnPk  = ncm_matrix_new (cbe->priv->psp.ln_tau_size, cbe->priv->psp.ln_k_size);
  guint i;

  for (i = 0; i < cbe->priv->psp.ln_k_size; i++)
  {
    ncm_vector_set (lnk_v, i, cbe->priv->psp.ln_k[i]);
    /*printf ("lnk[%u] % 20.15g k % 20.15g\n", i, cbe->priv->psp.ln_k[i], exp (cbe->priv->psp.ln_k[i]));*/
  }

  for (i = 0; i < cbe->priv->psp.ln_tau_size; i++)
  {
    const gdouble z_i = cbe->priv->psp.z_max_pk / (cbe->priv->psp.ln_tau_size - 1.0) * i;
    ncm_vector_set (z_v, i, z_i);
    spectra_pk_at_z (&cbe->priv->pba, &cbe->priv->psp, logarithmic, z_i, ncm_matrix_ptr (lnPk, i, 0), NULL);

/*    printf ("z % 20.15g\n", z_i);
    guint j;
    for (j = 0; j < cbe->priv->psp.ln_k_size; j++)
    {
      printf ("lnPk[%u] % 20.15g % 20.15g\n", j, ncm_matrix_get (lnPk, i, j), exp (ncm_matrix_get (lnPk, i, j)));
    }
*/
  }

  {
    NcmSpline2d *lnPk_s = ncm_spline2d_bicubic_notaknot_new ();
    ncm_spline2d_set (lnPk_s, lnk_v, z_v, lnPk, TRUE);

    ncm_vector_free (z_v);
    ncm_vector_free (lnk_v);
    ncm_matrix_free (lnPk);

    return lnPk_s;
  }
}

/**
 * nc_cbe_get_all_Cls:
 * @cbe: a #NcCBE
 * @TT_Cls: a #NcmVector
 * @EE_Cls: a #NcmVector
 * @BB_Cls: a #NcmVector
 * @TE_Cls: a #NcmVector
 * 
 * Gets and store the angular power spectra $C_l$'s calculated in the vectors @TT_Cls, @EE_Cls, 
 * @BB_Cls and @TE_Cls. If any of these vectors are NULL, then it is (they are)
 * ignored.
 * 
 */
void 
nc_cbe_get_all_Cls (NcCBE *cbe, NcmVector *TT_Cls, NcmVector *EE_Cls, NcmVector *BB_Cls, NcmVector *TE_Cls)
{
  guint all_Cls_size, index_tt, index_ee, index_bb, index_te;
  gboolean has_tt, has_ee, has_bb, has_te;

  if (cbe->use_lensed_Cls)
  {
    struct lensing *ptr = &cbe->priv->ple;
    all_Cls_size = ptr->lt_size;
    index_tt = ptr->index_lt_tt;
    index_ee = ptr->index_lt_ee;
    index_bb = ptr->index_lt_bb;
    index_te = ptr->index_lt_te;
    has_tt = ptr->has_tt;
    has_ee = ptr->has_ee;
    has_bb = ptr->has_bb;
    has_te = ptr->has_te;
  }
  else
  {
    struct spectra *ptr = &cbe->priv->psp;
    all_Cls_size = ptr->ct_size;
    index_tt = ptr->index_ct_tt;
    index_ee = ptr->index_ct_ee;
    index_bb = ptr->index_ct_bb;
    index_te = ptr->index_ct_te;
    has_tt = ptr->has_tt;
    has_ee = ptr->has_ee;
    has_bb = ptr->has_bb;
    has_te = ptr->has_te;
  }

  {
    guint TT_lmax = has_tt ? (TT_Cls != NULL ? ncm_vector_len (TT_Cls) : 0) : 0;
    guint EE_lmax = has_tt ? (EE_Cls != NULL ? ncm_vector_len (EE_Cls) : 0) : 0;
    guint BB_lmax = has_tt ? (BB_Cls != NULL ? ncm_vector_len (BB_Cls) : 0) : 0;
    guint TE_lmax = has_tt ? (TE_Cls != NULL ? ncm_vector_len (TE_Cls) : 0) : 0;
    const gdouble T_gamma0 = cbe->priv->pba.T_cmb;
    const gdouble Cl_fac   = gsl_pow_2 (1.0e6 * T_gamma0);
    gdouble *all_Cls       = g_new0 (gdouble, all_Cls_size);
    guint l;

    g_assert (!(cbe->target_Cls & NC_DATA_CMB_TYPE_TT) || has_tt);
    g_assert (!(cbe->target_Cls & NC_DATA_CMB_TYPE_EE) || has_ee);
    g_assert (!(cbe->target_Cls & NC_DATA_CMB_TYPE_BB) || has_bb);
    g_assert (!(cbe->target_Cls & NC_DATA_CMB_TYPE_TE) || has_te);

    for (l = 0; l <= cbe->scalar_lmax; l++)
    {
      if (cbe->use_lensed_Cls)
        lensing_cl_at_l (&cbe->priv->ple, l, all_Cls);
      else
        spectra_cl_at_l (&cbe->priv->psp, l, all_Cls, NULL, NULL);

      if (TT_lmax > 0)
      {
        const gdouble TT_Cl = all_Cls[index_tt];
        ncm_vector_set (TT_Cls, l, Cl_fac * TT_Cl);
        TT_lmax--;
      }
      if (EE_lmax > 0)
      {
        const gdouble EE_Cl = all_Cls[index_ee];
        ncm_vector_set (EE_Cls, l, Cl_fac * EE_Cl);
        EE_lmax--;
      }
      if (BB_lmax > 0)
      {
        const gdouble BB_Cl = all_Cls[index_bb];
        ncm_vector_set (BB_Cls, l, Cl_fac * BB_Cl);
        BB_lmax--;
      }
      if (TE_lmax > 0)
      {
        const gdouble TE_Cl = all_Cls[index_te];
        ncm_vector_set (TE_Cls, l, Cl_fac * TE_Cl);
        TE_lmax--;
      }
    }
    g_free (all_Cls);
  }
}
