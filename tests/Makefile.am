## Process this file with automake to produce Makefile.in

MY_CFLAGS = \
	 -DG_LOG_DOMAIN=\"NUMCOSMO\" \
	 $(GLIB_CFLAGS) \
	 $(SUNDIALS_CFLAGS) \
	 $(GSL_CFLAGS) \
	 $(FFTW3_CFLAGS) \
	 $(CFITSIO_CFLAGS) \
	 $(NLOPT_CFLAGS) \
	 -I$(top_srcdir)

AM_CFLAGS = \
	 -Wall \
	 $(MY_CFLAGS)

test_ncm_sparam_SOURCES =  \
	test_ncm_sparam.c

test_ncm_vector_SOURCES =  \
	test_ncm_vector.c

test_ncm_matrix_SOURCES =  \
	test_ncm_matrix.c

test_ncm_stats_vec_SOURCES =  \
	test_ncm_stats_vec.c
	
test_ncm_spline_SOURCES =  \
        test_ncm_spline.c

test_ncm_spline2d_SOURCES =  \
        test_ncm_spline2d.c

test_ncm_integral1d_SOURCES =  \
        test_ncm_integral1d.c

test_ncm_sf_sbessel_SOURCES =  \
	test_ncm_sf_sbessel.c

test_ncm_model_SOURCES =  \
        test_ncm_model.c \
	ncm_model_test.c \
	ncm_model_test.h

test_ncm_model_ctrl_SOURCES =  \
        test_ncm_model_ctrl.c

test_ncm_mset_SOURCES = \
	test_ncm_mset.c

test_ncm_obj_array_SOURCES = \
	test_ncm_obj_array.c

test_ncm_serialize_SOURCES = \
	test_ncm_serialize.c

test_ncm_data_gauss_cov_SOURCES = \
	test_ncm_data_gauss_cov.c \
	ncm_data_gauss_cov_test.c \
	ncm_data_gauss_cov_test.h

test_ncm_func_eval_SOURCES =  \
	test_ncm_func_eval.c

test_ncm_sphere_map_pix_SOURCES =  \
	test_ncm_sphere_map_pix.c

test_nc_hicosmo_de_SOURCES =  \
	test_nc_hicosmo_de.c

test_nc_window_SOURCES =  \
        test_nc_window.c
        
test_nc_transfer_func_SOURCES =  \
        test_nc_transfer_func.c        

test_nc_galaxy_acf_SOURCES =  \
	test_nc_galaxy_acf.c

test_nc_recomb_SOURCES =  \
	test_nc_recomb.c

test_nc_data_bao_rdv_SOURCES =  \
        test_nc_data_bao_rdv.c

test_nc_data_bao_dvdv_SOURCES =  \
        test_nc_data_bao_dvdv.c
        
test_nc_cluster_pseudo_counts_SOURCES =  \
        test_nc_cluster_pseudo_counts.c
        
check_PROGRAMS =  \
	test_ncm_vector               \
	test_ncm_matrix               \
	test_ncm_stats_vec            \
	test_ncm_spline               \
	test_ncm_spline2d             \
	test_ncm_integral1d           \
	test_ncm_sf_sbessel           \
	test_ncm_func_eval            \
	test_ncm_sparam               \
	test_ncm_model                \
	test_ncm_model_ctrl           \
	test_ncm_serialize            \
	test_ncm_mset                 \
	test_ncm_obj_array            \
	test_ncm_data_gauss_cov       \
	test_ncm_sphere_map_pix       \
	test_nc_hicosmo_de            \
	test_nc_window                \
	test_nc_transfer_func         \
	test_nc_galaxy_acf            \
	test_nc_recomb                \
	test_nc_data_bao_rdv          \
        test_nc_data_bao_dvdv         \
        test_nc_cluster_pseudo_counts

# TEST_PROGS += $(check_PROGRAMS)

test_ncm_sparam_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la 

test_ncm_vector_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la 

test_ncm_matrix_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la 

test_ncm_stats_vec_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la 

test_ncm_spline_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_spline2d_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_integral1d_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_sf_sbessel_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_model_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_model_ctrl_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_mset_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_obj_array_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_serialize_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_data_gauss_cov_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_ncm_func_eval_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la 

test_ncm_sphere_map_pix_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la 

test_nc_hicosmo_de_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_nc_window_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_nc_transfer_func_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_nc_galaxy_acf_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la 

test_nc_recomb_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la 

test_nc_data_bao_rdv_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_nc_data_bao_dvdv_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

test_nc_cluster_pseudo_counts_LDADD = $(top_builddir)/numcosmo/libnumcosmo.la

TESTS = $(check_PROGRAMS)

export VERBOSE = 1
