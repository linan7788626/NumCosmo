## Process this file with automake to produce Makefile.in

## Created by Anjuta

INCLUDES = 

AM_CFLAGS = \
	 -Wall \
	 -DG_LOG_DOMAIN=\"NUMCOSMO\" \
	 $(GLIB_CFLAGS) \
	 $(SUNDIALS_CFLAGS) \
	 $(GSL_CFLAGS) \
	 $(FFTW3_CFLAGS) \
	 -I$(top_srcdir)

SUBDIRS = 

lib_LTLIBRARIES = libnumcosmo.la

ncm_sources = \
	math/ncm_cfg.c                   \
	math/ncm_lapack.c                \
	math/ncm_vector.c                \
	math/ncm_matrix.c                \
	math/ncm_spline.c                \
	math/ncm_spline_gsl.c            \
	math/ncm_spline_cubic.c          \
	math/ncm_spline_cubic_notaknot.c \
	math/ncm_spline_func.c           \
	math/ncm_spline2d.c              \
	math/ncm_spline2d_spline.c       \
	math/ncm_spline2d_gsl.c          \
	math/ncm_spline2d_bicubic.c      \
	math/ncm_fftlog.c                \
	math/ncm_sparam.c                \
	math/ncm_vparam.c                \
	math/ncm_model.c                 \
	math/ncm_model_ctrl.c            \
	math/ncm_mset.c                  \
	math/ncm_mset_func.c             \
	math/ncm_ode_spline.c            \
	math/ncm_reparam.c               \
	math/ncm_reparam_linear.c        \
	math/ncm_fit.c

ncm_headers = \
	math/ncm_cfg.h                   \
	math/ncm_lapack.h                \
	math/ncm_vector.h                \
	math/ncm_matrix.h                \
	math/ncm_spline.h                \
	math/ncm_spline_gsl.h            \
	math/ncm_spline_cubic.h          \
	math/ncm_spline_cubic_notaknot.h \
	math/ncm_spline_func.h           \
	math/ncm_spline2d.h              \
	math/ncm_spline2d_spline.h       \
	math/ncm_spline2d_gsl.h          \
	math/ncm_spline2d_bicubic.h      \
	math/ncm_fftlog.h                \
	math/ncm_sparam.h                \
	math/ncm_vparam.h                \
	math/ncm_model.h                 \
	math/ncm_model_ctrl.h            \
	math/ncm_mset.h                  \
	math/ncm_mset_func.h             \
	math/ncm_ode_spline.h            \
	math/ncm_reparam.h               \
	math/ncm_reparam_linear.h        \
	math/ncm_fit.h

ncm_headers_internal = 

nc_headers = \
	nc_macros.h    \
	nc_constants.h \
	nc_hicosmo.h   \
	nc_distance.h  \
	model/nc_hicosmo_de.h        \
	model/nc_hicosmo_de_linder.h \
	model/nc_hicosmo_de_pad.h    \
	model/nc_hicosmo_de_qe.h     \
	model/nc_hicosmo_de_xcdm.h   \
	model/nc_hicosmo_lcdm.h      \
	model/nc_hicosmo_qconst.h    \
	model/nc_hicosmo_qlinear.h   \
	model/nc_hicosmo_qpw.h       \
	model/nc_hicosmo_qspline.h   \
	lss/nc_window.h             \
	lss/nc_window_tophat.h      \
	lss/nc_window_gaussian.h    \
	lss/nc_transfer_func.h      \
	lss/nc_transfer_func_bbks.h \
	lss/nc_transfer_func_eh.h   \
	lss/nc_transfer_func_camb.h \
	lss/nc_transfer_func_pert.h \
	lss/nc_growth_func.h        \
	lss/nc_matter_var.h         \
	lss/nc_galaxy_acf.h         \
	lss/nc_multiplicity_func.h  \
	lss/nc_multiplicity_func_ps.h \
	lss/nc_multiplicity_func_st.h \
	lss/nc_multiplicity_func_jenkins.h \
	lss/nc_multiplicity_func_warren.h \
	lss/nc_multiplicity_func_tinker.h \
	lss/nc_multiplicity_func_tinker_mean.h \
	lss/nc_multiplicity_func_tinker_crit.h \
	lss/nc_mass_function.h           \
        lss/nc_halo_bias_type.h          \
        lss/nc_halo_bias_type_ps.h       \
        lss/nc_halo_bias_type_st_spher.h \
        lss/nc_halo_bias_type_st_ellip.h \
        lss/nc_halo_bias_type_tinker.h   \
        lss/nc_halo_bias_func.h          \
        lss/nc_cluster_redshift.h            \
        lss/nc_cluster_redshift_nodist.h     \
        lss/nc_cluster_photoz_gauss.h        \
        lss/nc_cluster_photoz_gauss_global.h \
        lss/nc_cluster_mass.h                \
        lss/nc_cluster_mass_nodist.h         \
        lss/nc_cluster_mass_lnnormal.h       \
        lss/nc_cluster_abundance.h

nc_sources = \
	nc_hicosmo.c  \
	nc_distance.c \
	model/nc_hicosmo_de.c        \
	model/nc_hicosmo_de_linder.c \
	model/nc_hicosmo_de_pad.c    \
	model/nc_hicosmo_de_qe.c     \
	model/nc_hicosmo_de_xcdm.c   \
	model/nc_hicosmo_lcdm.c      \
	model/nc_hicosmo_qconst.c    \
	model/nc_hicosmo_qlinear.c   \
	model/nc_hicosmo_qpw.c       \
	model/nc_hicosmo_qspline.c   \
	lss/nc_window.c              \
	lss/nc_window_tophat.c       \
	lss/nc_window_gaussian.c     \
	lss/nc_transfer_func.c       \
	lss/nc_transfer_func_bbks.c  \
	lss/nc_transfer_func_eh.c    \
	lss/nc_transfer_func_camb.c  \
	lss/nc_transfer_func_pert.c  \
	lss/nc_growth_func.c         \
	lss/nc_matter_var.c          \
	lss/nc_galaxy_acf.c          \
	lss/nc_multiplicity_func.c   \
	lss/nc_multiplicity_func_ps.c \
	lss/nc_multiplicity_func_st.c \
	lss/nc_multiplicity_func_jenkins.c \
	lss/nc_multiplicity_func_warren.c \
	lss/nc_multiplicity_func_tinker.c \
	lss/nc_multiplicity_func_tinker_mean.c \
	lss/nc_multiplicity_func_tinker_crit.c \
	lss/nc_mass_function.c           \
	lss/nc_halo_bias_type.c          \
	lss/nc_halo_bias_type_ps.c       \
	lss/nc_halo_bias_type_st_spher.c \
	lss/nc_halo_bias_type_st_ellip.c \
	lss/nc_halo_bias_type_tinker.c   \
	lss/nc_halo_bias_func.c          \
	lss/nc_cluster_redshift.c            \
	lss/nc_cluster_redshift_nodist.c     \
	lss/nc_cluster_photoz_gauss.c        \
	lss/nc_cluster_photoz_gauss_global.c \
	lss/nc_cluster_mass.c                \
	lss/nc_cluster_mass_nodist.c         \
	lss/nc_cluster_mass_lnnormal.c       \
	lss/nc_cluster_abundance.c

libnumcosmo_la_SOURCES = \
	$(ncm_sources)   \
	$(ncm_headers)   \
	ncm_enum_types.c \
	ncm_enum_types.h \
	$(nc_sources)    \
	$(nc_headers)    \
	nc_enum_types.c  \
	nc_enum_types.h  \
	scalefactor.c \
	scalefactor.h \
	numcosmo.h    \
	data/data.c   \
	data/data.h   \
	data/data_gaussian.c         \
	data/data_gaussian.h         \
	data/data_onevardist.c       \
	data/data_onevardist.h       \
	data/data_poisson.c          \
	data/data_poisson.h          \
	data/dataset.c               \
	data/dataset.h               \
	data/data_distance_modulus.c \
	data/data_distance_modulus.h \
	data/data_hubble_function.c  \
	data/data_hubble_function.h  \
	data/data_baryonic_oscillation.h \
	data/data_baryonic_oscillation.c \
	data/data_cosmic_microwave_background.c \
	data/data_cosmic_microwave_background.h \
	model/quantum_gravity.c    \
	model/quantum_gravity.h    \
	likelihood/confidence_region.c \
	likelihood/least_squares.c     \
	likelihood/multimin.c          \
	likelihood/multimin_simplex.c  \
	likelihood/confidence_region.h \
	likelihood/least_squares.h     \
	likelihood/multimin.h          \
	likelihood/multimin_simplex.h  \
	likelihood/levmar.c  \
	likelihood/levmar.h  \
	likelihood/nc_nlopt.c   \
	likelihood/nc_nlopt.h   \
	likelihood/likelihood.c \
	likelihood/likelihood.h \
	likelihood/priors.c     \
	likelihood/priors.h     \
	math/poly.c               \
	math/integral.c       \
	math/function_cache.c \
	math/quaternion.c     \
	math/cvode_util.c     \
	math/quadrature.c     \
	math/matrix_exp.c     \
	math/magnus_iserles_ode.c \
	math/cvode_util.h         \
	math/function_cache.h     \
	math/integral.h           \
	math/magnus_iserles_ode.h \
	math/matrix_exp.h         \
	math/poly.h               \
	math/quadrature.h         \
	math/quaternion.h         \
	math/binsplit.c           \
	math/binsplit.h           \
	math/trig_integral.c      \
	math/trig_integral.h      \
	math/mpq_tree.c           \
	math/mpq_tree.h           \
	math/grid_one.c           \
	math/grid_one.h           \
	math/mp_spherical_bessel.c \
	math/mp_spherical_bessel.h \
	math/spherical_bessel.c    \
	math/spherical_bessel.h    \
	math/spherical_bessel_integral.c \
	math/spherical_bessel_integral.h \
	math/mp_spherical_bessel_integral.c \
	math/mp_spherical_bessel_integral.h \
	math/mp_hypergeometric_0F1.c \
	math/mp_hypergeometric_0F1.h \
	math/dividedifference.c \
	math/dividedifference.h \
	math/util.c             \
	math/util.h             \
	math/function_eval.c    \
	math/function_eval.h    \
	math/memory_pool.c      \
	math/memory_pool.h      \
	sphere/map.c            \
	sphere/healpix.c        \
	sphere/healpix.h        \
	sphere/map.h            \
	thermodyn/recomb.c      \
	thermodyn/recomb.h      \
	perturbations/linear.c  \
	perturbations/linear.h  \
	perturbations/covariance.c      \
	perturbations/covariance.h      \
	perturbations/linear_internal.h \
	perturbations/linear_cvodes.c   \
	perturbations/linear_gsl_ode.c  \
	perturbations/linear_gsl_ode2.c \
	perturbations/hydrodyn_adiabatic.c \
	perturbations/hydrodyn_adiabatic.h \
	lss/catalog_parser.c \
	lss/catalog_parser.h \
	lss/read_matrix.c \
	lss/read_matrix.h \
	lss/data_cluster_abundance.c \
	lss/data_cluster_abundance.h \
	lss/print_data.c \
	lss/print_data.h

libnumcosmo_la_LIBADD = \
	$(SUNDIALS_LIBS) \
	$(FFTW3_LIBS) \
	$(GLIB_LIBS) \
	$(NLOPT_LIBS) \
	$(GSL_LIBS) \
	$(SQLITE3_LIBS)

libnumcosmo_la_LDFLAGS = -version-info $(NUMCOSMO_LT_VERSION)

AM_CPPFLAGS = \
	$(FFTW3_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NLOPT_CFLAGS) \
	$(GSL_CFLAGS) \
	$(SQLITE3_CFLAGS)

nobase_cosmo_include_HEADERS =  \
	$(ncm_headers)          \
	$(ncm_headers_internal) \
	ncm_enum_types.h        \
	$(nc_headers)           \
	nc_enum_types.h         \
	numcosmo.h              \
	scalefactor.h           \
	build_cfg.h             \
	data/data.h             \
	data/data_gaussian.h         \
	data/data_onevardist.h       \
	data/data_poisson.h          \
	data/dataset.h               \
	data/data_distance_modulus.h \
	data/data_hubble_function.h  \
	data/data_baryonic_oscillation.h        \
	data/data_cosmic_microwave_background.h \
	model/quantum_gravity.h    \
	likelihood/confidence_region.h \
	likelihood/least_squares.h     \
	likelihood/multimin.h          \
	likelihood/multimin_simplex.h  \
	likelihood/levmar.h            \
	likelihood/nc_nlopt.h          \
	likelihood/likelihood.h        \
	likelihood/priors.h            \
	math/cvode_util.h         \
	math/function_cache.h     \
	math/integral.h           \
	math/magnus_iserles_ode.h \
	math/matrix_exp.h         \
	math/poly.h               \
	math/quadrature.h         \
	math/quaternion.h         \
	math/binsplit.h           \
	math/binsplit_eval.c      \
	math/trig_integral.h      \
	math/mpq_tree.h           \
	math/grid_one.h           \
	math/mp_spherical_bessel.h          \
	math/spherical_bessel.h             \
	math/spherical_bessel_integral.h    \
	math/mp_spherical_bessel_integral.h \
	math/mp_hypergeometric_0F1.h        \
	math/dividedifference.h \
	math/util.h             \
	math/function_eval.h    \
	math/memory_pool.h      \
	sphere/healpix.h        \
	sphere/map.h            \
	thermodyn/recomb.h      \
	perturbations/linear.h  \
	perturbations/linear_generic.c     \
	perturbations/covariance.h         \
	perturbations/linear_internal.h    \
	perturbations/hydrodyn_adiabatic.h \
	lss/catalog_parser.h \
	lss/read_matrix.h \
	lss/data_cluster_abundance.h \
	lss/print_data.h

cosmo_includedir = \
	$(pkgincludedir)

ncm_enum_types.h: $(ncm_headers) ncm_enum_types.h.template
	$(AM_V_GEN) ( cd $(srcdir) && $(GLIB_MKENUMS) --template ncm_enum_types.h.template $(ncm_headers) ) > \
	  ncm_enum_types.h.tmp && mv ncm_enum_types.h.tmp ncm_enum_types.h

ncm_enum_types.c: $(ncm_headers) ncm_enum_types.c.template
	$(AM_V_GEN) ( cd $(srcdir) && $(GLIB_MKENUMS) --template ncm_enum_types.c.template $(ncm_headers) ) > \
	  ncm_enum_types.c.tmp && mv ncm_enum_types.c.tmp ncm_enum_types.c

nc_enum_types.h: $(nc_headers) nc_enum_types.h.template
	$(AM_V_GEN) ( cd $(srcdir) && $(GLIB_MKENUMS) --template nc_enum_types.h.template $(nc_headers) ) > \
	  nc_enum_types.h.tmp && mv nc_enum_types.h.tmp nc_enum_types.h

nc_enum_types.c: $(nc_headers) nc_enum_types.c.template
	$(AM_V_GEN) ( cd $(srcdir) && $(GLIB_MKENUMS) --template nc_enum_types.c.template $(nc_headers) ) > \
	  nc_enum_types.c.tmp && mv nc_enum_types.c.tmp nc_enum_types.c

# these sources (also mentioned above) are generated.
BUILT_SOURCES =          \
        ncm_enum_types.h \
        ncm_enum_types.c \
        nc_enum_types.h  \
        nc_enum_types.c

EXTRA_DIST = \
	$(nobase_cosmo_include_HEADERS) \
	build_cfg.h.in            \
	ncm_enum_types.h.template \
	ncm_enum_types.h          \
	ncm_enum_types.c.template \
	ncm_enum_types.c          \
	nc_enum_types.h.template  \
	nc_enum_types.h           \
	nc_enum_types.c.template  \
	nc_enum_types.c

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) -I$(srcdir)/../ -I$(top_builddir) \
  --identifier-prefix=Ncm --identifier-prefix=Nc --warn-all

INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)
#Numcosmo_1_0_gir_NAMESPACE=Nc
#Numcosmo_1_0_gir_VERSION=1.0

if HAVE_INTROSPECTION

introspection_sources = numcosmo.h $(libnumcosmo_la_SOURCES)

Numcosmo-1.0.gir: libnumcosmo.la
Numcosmo_1_0_gir_INCLUDES = GObject-2.0
Numcosmo_1_0_gir_CFLAGS = $(INCLUDES)
Numcosmo_1_0_gir_LIBS = libnumcosmo.la
Numcosmo_1_0_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += Numcosmo-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)
  
typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)
      
CLEANFILES = $(gir_DATA) $(typelib_DATA)

endif