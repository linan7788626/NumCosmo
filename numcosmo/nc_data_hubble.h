/***************************************************************************
 *            nc_data_hubble.h
 *
 *  Thu Apr 22 14:35:37 2010
 *  Copyright  2010  Sandro Dias Pinto Vitenti
 *  <sandro@isoftware.com.br>
 ****************************************************************************/
/*
 * numcosmo
 * Copyright (C) 2012 Sandro Dias Pinto Vitenti <sandro@isoftware.com.br>
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

#ifndef _NC_DATA_HUBBLE_H_
#define _NC_DATA_HUBBLE_H_

#include <glib.h>
#include <glib-object.h>
#include <numcosmo/math/ncm_data_gauss_diag.h>

G_BEGIN_DECLS

#define NC_TYPE_DATA_HUBBLE             (nc_data_hubble_get_type ())
#define NC_DATA_HUBBLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NC_TYPE_DATA_HUBBLE, NcDataHubble))
#define NC_DATA_HUBBLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NC_TYPE_DATA_HUBBLE, NcDataHubbleClass))
#define NC_IS_DATA_HUBBLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NC_TYPE_DATA_HUBBLE))
#define NC_IS_DATA_HUBBLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NC_TYPE_DATA_HUBBLE))
#define NC_DATA_HUBBLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NC_TYPE_DATA_HUBBLE, NcDataHubbleClass))

typedef struct _NcDataHubbleClass NcDataHubbleClass;
typedef struct _NcDataHubble NcDataHubble;

/**
 * NcDataHubbleId:
 * @NC_DATA_HUBBLE_VERDE: FIXME
 * @NC_DATA_HUBBLE_CABRE: FIXME
 *
 * FIXME
 */
typedef enum _NcDataHubbleId
{
  NC_DATA_HUBBLE_VERDE = 0,
  NC_DATA_HUBBLE_CABRE,    /*< private >*/
  NC_DATA_HUBBLE_NSAMPLES, /*< skip >*/
} NcDataHubbleId;

struct _NcDataHubbleClass
{
  /*< private >*/
  NcmDataGaussDiagClass parent_class;
};

struct _NcDataHubble
{
  /*< private >*/
  NcmDataGaussDiag parent_instance;
  NcmVector *x;
  NcDataHubbleId id;
};

GType nc_data_hubble_get_type (void) G_GNUC_CONST;

NcmData *nc_data_hubble_new (NcDataHubbleId id);

void nc_data_hubble_set_size (NcDataHubble *hubble, guint np);
guint nc_data_hubble_get_size (NcDataHubble *hubble);

void nc_data_hubble_set_sample (NcDataHubble *hubble, NcDataHubbleId id);
NcDataHubbleId nc_data_hubble_get_sample (NcDataHubble *hubble);

G_END_DECLS

#endif /* _NC_DATA_HUBBLE_H_ */
