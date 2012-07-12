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

#ifndef EGG_PIECEWISE_LINEAR_VIEW_H
#define EGG_PIECEWISE_LINEAR_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EGG_TYPE_PIECEWISE_LINEAR_VIEW             (egg_piecewise_linear_view_get_type())
#define EGG_PIECEWISE_LINEAR_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), EGG_TYPE_PIECEWISE_LINEAR_VIEW, EggPiecewiseLinearView))
#define EGG_IS_PIECEWISE_LINEAR_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), EGG_TYPE_PIECEWISE_LINEAR_VIEW))
#define EGG_PIECEWISE_LINEAR_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), EGG_TYPE_PIECEWISE_LINEAR_VIEW, EggPiecewiseLinearViewClass))
#define EGG_IS_PIECEWISE_LINEAR_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), EGG_TYPE_PIECEWISE_LINEAR_VIEW))
#define EGG_PIECEWISE_LINEAR_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), EGG_TYPE_PIECEWISE_LINEAR_VIEW, EggPiecewiseLinearViewClass))

typedef struct _EggPiecewiseLinearView           EggPiecewiseLinearView;
typedef struct _EggPiecewiseLinearViewClass      EggPiecewiseLinearViewClass;
typedef struct _EggPiecewiseLinearViewPrivate    EggPiecewiseLinearViewPrivate;

struct _EggPiecewiseLinearView
{
    GtkDrawingArea  parent_instance;

    /*< private >*/
    EggPiecewiseLinearViewPrivate *priv;
};

struct _EggPiecewiseLinearViewClass
{
    GtkDrawingAreaClass     parent_class;

    /* signals */
    void (* point_changed)  (EggPiecewiseLinearView *view, guint index, gint value);
};

GType       egg_piecewise_linear_view_get_type              (void);
GtkWidget * egg_piecewise_linear_view_new                   (guint                   n_points,
                                                             gint                    min,
                                                             gint                    max);
void        egg_piecewise_linear_view_set_number_of_points  (EggPiecewiseLinearView *view,
                                                             guint                   n_points);
void        egg_piecewise_linear_view_set_point             (EggPiecewiseLinearView *view, 
                                                             guint                   index, 
                                                             gint                    value);
void        egg_piecewise_linear_view_set_range             (EggPiecewiseLinearView *view, 
                                                             gint                    min, 
                                                             gint                    max);
void        egg_piecewise_linear_view_get_point             (EggPiecewiseLinearView *view,
                                                             guint                   index,
                                                             gint                   *x,
                                                             gint                   *y);

G_END_DECLS

#endif
