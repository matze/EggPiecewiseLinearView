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

#include <stdlib.h>
#include <math.h>
#include "egg-data-points.h"

G_DEFINE_TYPE (EggDataPoints, egg_data_points, G_TYPE_OBJECT)

#define EGG_DATA_POINTS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), EGG_TYPE_DATA_POINTS, EggDataPointsPrivate))

struct _EggDataPointsPrivate
{
    gdouble      lower_x, upper_x;
    gdouble      lower_y, upper_y;
    GPtrArray   *x_adjustments;
    GPtrArray   *y_adjustments;
};

enum
{
    PROP_0,
    PROP_LOWER_X,
    PROP_UPPER_X,
    PROP_LOWER_Y,
    PROP_UPPER_Y,
    N_PROPERTIES
};

enum
{
    POINT_CHANGED,
    LAST_SIGNAL
};

static GParamSpec *egg_data_points_properties[N_PROPERTIES] = { NULL, };

static guint egg_data_points_signals[LAST_SIGNAL] = { 0 };


EggDataPoints *
egg_data_points_new (gdouble lower_x, gdouble upper_x,
                     gdouble lower_y, gdouble upper_y)
{
    GObject *object;

    object = g_object_new (EGG_TYPE_DATA_POINTS,
                           "lower-x", lower_x,
                           "upper-x", upper_x,
                           "lower-y", lower_y,
                           "upper-y", upper_y,
                           NULL);

    return EGG_DATA_POINTS (object);
}

void
egg_data_points_get_x_range (EggDataPoints *points,
                             gdouble       *lower_x,
                             gdouble       *upper_x)
{
    g_return_if_fail (EGG_IS_DATA_POINTS (points));
    *lower_x = points->priv->lower_x;
    *upper_x = points->priv->upper_x;
}

void
egg_data_points_get_y_range (EggDataPoints *points,
                             gdouble       *lower_y,
                             gdouble       *upper_y)
{

    g_return_if_fail (EGG_IS_DATA_POINTS (points));
    *lower_y = points->priv->lower_y;
    *upper_y = points->priv->upper_y;
}

void
egg_data_points_add_point (EggDataPoints *data_points,
                           gdouble        x,
                           gdouble        y,
                           gdouble        increment)
{
    EggDataPointsPrivate *priv;
    GtkAdjustment *x_adj;
    GtkAdjustment *y_adj;

    g_return_if_fail (EGG_IS_DATA_POINTS (data_points));

    priv  = EGG_DATA_POINTS_GET_PRIVATE (data_points);
    x_adj = GTK_ADJUSTMENT (gtk_adjustment_new (x, priv->lower_x, priv->upper_x, increment, 10, 0));
    y_adj = GTK_ADJUSTMENT (gtk_adjustment_new (y, priv->lower_y, priv->upper_y, increment, 10, 0));

    g_ptr_array_add (priv->x_adjustments, x_adj);
    g_ptr_array_add (priv->y_adjustments, y_adj);
}

guint
egg_data_points_get_num (EggDataPoints *points)
{
    g_return_val_if_fail (EGG_IS_DATA_POINTS (points), 0);
    return points->priv->x_adjustments->len;
}

GtkAdjustment *
egg_data_points_get_x (EggDataPoints *data_points,
                       guint          index)
{
    EggDataPointsPrivate *priv;

    g_return_val_if_fail (EGG_IS_DATA_POINTS (data_points), NULL);

    priv = EGG_DATA_POINTS_GET_PRIVATE (data_points);
    g_return_val_if_fail (index < priv->x_adjustments->len, NULL);

    return GTK_ADJUSTMENT (g_ptr_array_index (priv->x_adjustments, index));
}

GtkAdjustment *
egg_data_points_get_y (EggDataPoints *data_points,
                       guint          index)
{
    EggDataPointsPrivate *priv;

    g_return_val_if_fail (EGG_IS_DATA_POINTS (data_points), NULL);

    priv = EGG_DATA_POINTS_GET_PRIVATE (data_points);
    g_return_val_if_fail (index < priv->y_adjustments->len, NULL);

    return GTK_ADJUSTMENT (g_ptr_array_index (priv->y_adjustments, index));
}

gdouble egg_data_points_get_x_value (EggDataPoints *data_points,
                                     guint          index)
{
    EggDataPointsPrivate *priv;
    GtkAdjustment        *adj;

    g_return_val_if_fail (EGG_IS_DATA_POINTS (data_points), 0.0);

    priv = EGG_DATA_POINTS_GET_PRIVATE (data_points);
    g_return_val_if_fail (index < priv->x_adjustments->len, 0.0);

    adj = GTK_ADJUSTMENT (g_ptr_array_index (priv->x_adjustments, index));
    return gtk_adjustment_get_value (adj);
}

gdouble egg_data_points_get_y_value (EggDataPoints *data_points,
                                     guint          index)
{
    EggDataPointsPrivate *priv;
    GtkAdjustment        *adj;

    g_return_val_if_fail (EGG_IS_DATA_POINTS (data_points), 0.0);

    priv = EGG_DATA_POINTS_GET_PRIVATE (data_points);
    g_return_val_if_fail (index < priv->y_adjustments->len, 0.0);

    adj = GTK_ADJUSTMENT (g_ptr_array_index (priv->y_adjustments, index));
    return gtk_adjustment_get_value (adj);
}

void
egg_data_points_set_x (EggDataPoints *data_points,
                       guint          index,
                       gdouble        value)
{
    EggDataPointsPrivate *priv;
    GtkAdjustment        *adj;

    g_return_if_fail (EGG_IS_DATA_POINTS (data_points));

    priv = EGG_DATA_POINTS_GET_PRIVATE (data_points);
    g_return_if_fail (index < priv->x_adjustments->len);

    adj = g_ptr_array_index (priv->x_adjustments, index);
    gtk_adjustment_set_value (adj, value);
}

void
egg_data_points_set_y (EggDataPoints *data_points,
                       guint          index,
                       gdouble        value)
{
    EggDataPointsPrivate *priv;
    GtkAdjustment        *adj;

    g_return_if_fail (EGG_IS_DATA_POINTS (data_points));

    priv = EGG_DATA_POINTS_GET_PRIVATE (data_points);
    g_return_if_fail (index < priv->x_adjustments->len);

    adj = g_ptr_array_index (priv->x_adjustments, index);
    gtk_adjustment_set_value (adj, value);
}

guint
egg_data_get_closest_point (EggDataPoints *points,
                            gdouble        x,
                            gdouble        y,
                            gdouble       *distance)
{
    EggDataPointsPrivate *priv;
    guint   index        = 0;
    gdouble min_distance = DBL_MAX;
    
    g_return_val_if_fail (EGG_IS_DATA_POINTS (points), 0);
    priv = EGG_DATA_POINTS_GET_PRIVATE (points);

    for (guint i = 0; i < priv->x_adjustments->len; i++) {
        GtkAdjustment *x_adj;
        GtkAdjustment *y_adj;
        gdouble d, xp, yp;
        
        x_adj = g_ptr_array_index (priv->x_adjustments, i);
        y_adj = g_ptr_array_index (priv->y_adjustments, i);
        xp = x - gtk_adjustment_get_value (x_adj);
        yp = y - gtk_adjustment_get_value (y_adj);
        xp /= (priv->upper_x - priv->lower_x);
        yp /= (priv->upper_y - priv->lower_y);
        d = sqrt (xp*xp + yp*yp);

        if (d < min_distance) {
            index = i; 
            min_distance = d;
        }
    }

    *distance = min_distance;
    return index;
}

static void
egg_data_points_set_property (GObject        *object,
                              guint           property_id,
                              const GValue   *value,
                              GParamSpec     *pspec)
{
    EggDataPointsPrivate *priv;

    g_return_if_fail (EGG_IS_DATA_POINTS (object));
    priv = EGG_DATA_POINTS_GET_PRIVATE (object);

    switch (property_id) {
        case PROP_LOWER_X:
            priv->lower_x = g_value_get_double (value);
            break;
        case PROP_UPPER_X:
            priv->upper_x = g_value_get_double (value);
            break;
        case PROP_LOWER_Y:
            priv->lower_y = g_value_get_double (value);
            break;
        case PROP_UPPER_Y:
            priv->upper_y = g_value_get_double (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            return;
    }
}

static void
egg_data_points_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    EggDataPointsPrivate *priv;

    g_return_if_fail (EGG_IS_DATA_POINTS (object));
    priv = EGG_DATA_POINTS_GET_PRIVATE (object);

    switch (property_id) {
        case PROP_LOWER_X:
            g_value_set_double (value, priv->lower_x);
            break;
        case PROP_UPPER_X:
            g_value_set_double (value, priv->upper_x);
            break;
        case PROP_LOWER_Y:
            g_value_set_double (value, priv->lower_y);
            break;
        case PROP_UPPER_Y:
            g_value_set_double (value, priv->upper_y);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            return;
    }
}

static void
egg_data_points_class_init (EggDataPointsClass *klass)
{
    GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = egg_data_points_set_property;
    gobject_class->get_property = egg_data_points_get_property;

    egg_data_points_properties[PROP_LOWER_X] =
        g_param_spec_double ("lower-x",
                             "Lower bound of the x data range",
                             "Lower bound of the x data range",
                             -DBL_MAX, DBL_MAX, 0.0,
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    egg_data_points_properties[PROP_UPPER_X] =
        g_param_spec_double ("upper-x",
                             "Upper bound of the x data range",
                             "Upper bound of the x data range",
                             -DBL_MAX, DBL_MAX, 1.0,
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    egg_data_points_properties[PROP_LOWER_Y] =
        g_param_spec_double ("lower-y",
                             "Lower bound of the y data range",
                             "Lower bound of the y data range",
                             -DBL_MAX, DBL_MAX, 0.0,
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    egg_data_points_properties[PROP_UPPER_Y] =
        g_param_spec_double ("upper-y",
                             "Upper bound of the y data range",
                             "Upper bound of the y data range",
                             -DBL_MAX, DBL_MAX, 1.0,
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES,
                                       egg_data_points_properties);

    egg_data_points_signals[POINT_CHANGED] =
        g_signal_new ("point-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE,
                      1, G_TYPE_UINT);

    g_type_class_add_private (klass, sizeof (EggDataPointsPrivate));
}

static void
egg_data_points_init (EggDataPoints *points)
{
    points->priv = EGG_DATA_POINTS_GET_PRIVATE (points);
    points->priv->x_adjustments = g_ptr_array_new ();
    points->priv->y_adjustments = g_ptr_array_new ();
}