/* Copyright (C) 2011, 2012 Matthias Vogelgesang <matthias.vogelgesang@kit.edu>
   (Karlsruhe Institute of Technology)

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by the
   Free Software Foundation; either version 2.1 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
   details.

   You should have received a copy of the GNU Lesser General Public License along
   with this library; if not, write to the Free Software Foundation, Inc., 51
   Franklin St, Fifth Floor, Boston, MA 02110, USA */

#ifndef EGG_DATA_POINTS_H
#define EGG_DATA_POINTS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EGG_TYPE_DATA_POINTS             (egg_data_points_get_type())
#define EGG_DATA_POINTS(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), EGG_TYPE_DATA_POINTS, EggDataPoints))
#define EGG_IS_DATA_POINTS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), EGG_TYPE_DATA_POINTS))
#define EGG_DATA_POINTS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), EGG_TYPE_DATA_POINTS, EggDataPointsClass))
#define EGG_IS_DATA_POINTS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), EGG_TYPE_DATA_POINTS))
#define EGG_DATA_POINTS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), EGG_TYPE_DATA_POINTS, EggDataPointsClass))

typedef struct _EggDataPoints           EggDataPoints;
typedef struct _EggDataPointsClass      EggDataPointsClass;
typedef struct _EggDataPointsPrivate    EggDataPointsPrivate;

struct _EggDataPoints
{
    GObject parent_instance;

    /*< private >*/
    EggDataPointsPrivate *priv;
};

struct _EggDataPointsClass
{
    GObjectClass parent_class;

    /* signals */
    void (* point_changed)  (EggDataPoints *view, guint index);
};

GType             egg_data_points_get_type  (void);
EggDataPoints   * egg_data_points_new       (gdouble        lower_x,
                                             gdouble        upper_x,
                                             gdouble        lower_y,
                                             gdouble        upper_y);
void              egg_data_points_get_x_range (EggDataPoints *data_points,
                                               gdouble       *lower_x,
                                               gdouble       *upper_x);
void              egg_data_points_get_y_range (EggDataPoints *data_points,
                                               gdouble       *lower_y,
                                               gdouble       *upper_y);
void              egg_data_points_add_point (EggDataPoints *data_points,
                                             gdouble        x,
                                             gdouble        y,
                                             gdouble        increment);
guint             egg_data_points_get_num   (EggDataPoints *data_points);
GtkAdjustment   * egg_data_points_get_x     (EggDataPoints *data_points,
                                             guint          index);
GtkAdjustment   * egg_data_points_get_y     (EggDataPoints *data_points,
                                             guint          index);
gdouble           egg_data_points_get_x_value (EggDataPoints *data_points,
                                               guint          index);
gdouble           egg_data_points_get_y_value (EggDataPoints *data_points,
                                               guint          index);
void              egg_data_points_set_x     (EggDataPoints *data_points,
                                             guint          index,
                                             gdouble        value);
void              egg_data_points_set_y     (EggDataPoints *data_points,
                                             guint          index,
                                             gdouble        value);
guint             egg_data_get_closest_point (EggDataPoints *data_points,
                                              gdouble        x,
                                              gdouble        y,
                                              gdouble       *distance);

G_END_DECLS

#endif
