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
#include "egg-piecewise-linear-view.h"

G_DEFINE_TYPE (EggPiecewiseLinearView, egg_piecewise_linear_view, GTK_TYPE_DRAWING_AREA)

#define EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), EGG_TYPE_PIECEWISE_LINEAR_VIEW, EggPiecewiseLinearViewPrivate))

#define MIN_WIDTH   128
#define MIN_HEIGHT  128

struct _EggPiecewiseLinearViewPrivate
{
    gint            border_width;
    guint           n_points;
    guint          *points;
    guint           dragged;
    gint            min, max;
    gboolean        grabbed;
    GdkCursorType   cursor_type;
};

enum
{
    PROP_0,
    PROP_NUM_POINTS,
    PROP_MIN,
    PROP_MAX,
    N_PROPERTIES
};

enum
{
    POINT_CHANGED,
    LAST_SIGNAL
};

static GParamSpec *egg_piecewise_linear_view_properties[N_PROPERTIES] = { NULL, };

static guint egg_piecewise_linear_view_signals[LAST_SIGNAL] = { 0 };

GtkWidget *
egg_piecewise_linear_view_new (guint n_points, gint min, gint max)
{
    GObject *view;

    view = g_object_new (EGG_TYPE_PIECEWISE_LINEAR_VIEW, 
                         "num-points", n_points, 
                         "min", min,
                         "max", max,
                         NULL);

    return GTK_WIDGET (view);
}

void
egg_piecewise_linear_view_set_number_of_points (EggPiecewiseLinearView *view, guint n_points)
{
    g_return_if_fail (EGG_PIECEWISE_LINEAR_VIEW (view));
    g_object_set (G_OBJECT (view), "num-points", n_points, NULL);
    gtk_widget_queue_draw (GTK_WIDGET (view));
}

static void
clamp_points (EggPiecewiseLinearViewPrivate *priv)
{
    for (guint i = 0; i < priv->n_points; i++)
        priv->points[i] = CLAMP (priv->points[i], priv->min, priv->max);
}

void
egg_piecewise_linear_view_set_range (EggPiecewiseLinearView *view, gint min, gint max)
{
    g_return_if_fail (EGG_PIECEWISE_LINEAR_VIEW (view));

    g_object_set (G_OBJECT (view),
            "min", min,
            "max", max,
            NULL);
}

void
egg_piecewise_linear_view_set_point (EggPiecewiseLinearView *view, guint index, gint value)
{
    g_return_if_fail (EGG_PIECEWISE_LINEAR_VIEW (view));

    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (view);

    g_return_if_fail (index < priv->n_points);
    g_return_if_fail (value < priv->max);

    priv->points[index] = value;
}

void egg_piecewise_linear_view_get_point (EggPiecewiseLinearView *view, guint index, gint *x, gint *y)
{
    g_return_if_fail (EGG_PIECEWISE_LINEAR_VIEW (view));

    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (view);

    g_return_if_fail (index < priv->n_points);
    *y = priv->points[index];
    *x = 0;
}

static void
egg_piecewise_linear_view_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
    /* TODO: maybe add border width */
    requisition->width = MIN_WIDTH;
    requisition->height = MIN_HEIGHT;
}

static gdouble
map_point (guint y, guint max, guint height)
{
    return height - height * y / ((gdouble) max);
}

static gboolean
egg_piecewise_linear_view_expose (GtkWidget *widget, GdkEventExpose *event)
{
    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (widget);
    GtkStyle        *style = gtk_widget_get_style (widget);
    GtkAllocation    allocation;
    cairo_t         *cr;
    gint             border;
    gint             width, height;
    gdouble          xshift;

    cr = gdk_cairo_create (gtk_widget_get_window (widget));
    gdk_cairo_region (cr, event->region);
    cairo_clip (cr);

    /* Draw the background */
    gdk_cairo_set_source_color (cr, &style->base[GTK_STATE_NORMAL]);
    cairo_paint (cr);

    gtk_widget_get_allocation (widget, &allocation);
    border = priv->border_width;
    width  = allocation.width - 2 * border;
    height = allocation.height - 2 * border;

    gdk_cairo_set_source_color (cr, &style->dark[GTK_STATE_NORMAL]);
    cairo_set_line_width (cr, 1.0);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
    cairo_translate (cr, 0.5, 0.5);
    cairo_rectangle (cr, border, border, width - 1, height - 1);
    cairo_stroke (cr);

    /* Draw lines */
    xshift = width / ((gdouble) priv->n_points - 1);
    cairo_set_line_width (cr, 1.5);
    cairo_move_to (cr, border, map_point (priv->points[0], priv->max, height));

    for (guint i = 1; i < priv->n_points; i++)
        cairo_line_to (cr, border + i*xshift, map_point (priv->points[i], priv->max, height));

    cairo_stroke (cr);

    /* Draw points */
    for (guint i = 0; i < priv->n_points; i++) {
        gdouble y = map_point (priv->points[i], priv->max, height);
        cairo_move_to (cr, border + i*xshift + 3, y);
        cairo_arc (cr, border + i*xshift, y, 3, 0, 2 * G_PI);
        cairo_stroke (cr);
    }

    cairo_destroy (cr);
    return FALSE;
}

static void
set_cursor_type (EggPiecewiseLinearView *view, GdkCursorType cursor_type)
{
    if (cursor_type != view->priv->cursor_type) {
        GdkCursor *cursor = gdk_cursor_new (cursor_type);

        gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET(view)), cursor);
        gdk_cursor_unref (cursor);
        view->priv->cursor_type = cursor_type;
    }
}

static gboolean
egg_piecewise_linear_button_press (GtkWidget *widget, GdkEventButton *event)
{
    EggPiecewiseLinearView
                    *view = EGG_PIECEWISE_LINEAR_VIEW (widget);
    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (view);
    GtkAllocation    allocation;
    gint             border;
    gint             width;
    gdouble          x, rx;

    if (event->button != 1)
        return TRUE;

    gtk_widget_get_allocation (widget, &allocation);

    border  = priv->border_width;
    width   = allocation.width - 2 * border;
    x       = (gdouble) (event->x - border) / (gdouble) width;
    rx      = floor(x * (priv->n_points - 1) + 0.5);

    if (ABS ((x * (priv->n_points - 1)) - rx) < 0.1) {
        priv->dragged = (guint) rx;
        g_assert (priv->dragged < priv->n_points);
    }

    view->priv->grabbed = TRUE;
    set_cursor_type (view, GDK_FLEUR);

    return TRUE;
}

static gboolean
egg_piecewise_linear_button_release (GtkWidget *widget, GdkEventButton *event)
{
    EggPiecewiseLinearView
                    *view = EGG_PIECEWISE_LINEAR_VIEW (widget);
    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (view);

    if (event->button != 1)
        return TRUE;

    priv->grabbed = FALSE;
    set_cursor_type (view, GDK_TCROSS);

    g_signal_emit (view, egg_piecewise_linear_view_signals[POINT_CHANGED], 0, priv->dragged, priv->points[priv->dragged]);

    return TRUE;
}

static gboolean
egg_piecewise_linear_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (widget);
    GtkAllocation    allocation;
    GdkCursorType    cursor_type = GDK_TCROSS;
    gint             border;
    gint             width;
    gint             height;
    gdouble          x;
    gdouble          rounded_x;

    gtk_widget_get_allocation (widget, &allocation);
    border = priv->border_width;
    width  = allocation.width - 2 * border;
    height = allocation.height - 2 * border;
    x = (gdouble) (event->x - border) / (gdouble) width;

    rounded_x = floor(x * (priv->n_points - 1) + 0.5);

    if (!priv->grabbed) {
        if (ABS ((x * (priv->n_points - 1)) - rounded_x) < 0.1) {
            cursor_type = GDK_FLEUR;
        }
    }
    else {
        gdouble y;

        y = (gdouble) (height - event->y - border) / (gdouble) height;
        y = CLAMP (y, 0.0, 1.0);
        y *= priv->max;
        priv->points[priv->dragged] = (gint) y;

        cursor_type = GDK_FLEUR;
    }

    set_cursor_type (EGG_PIECEWISE_LINEAR_VIEW (widget), cursor_type);
    gtk_widget_queue_draw (widget);

    return TRUE;
}

static void
egg_piecewise_linear_view_set_property (GObject        *object,
                                        guint           property_id,
                                        const GValue   *value,
                                        GParamSpec     *pspec)
{
    EggPiecewiseLinearViewPrivate *priv;

    g_return_if_fail (EGG_IS_PIECEWISE_LINEAR_VIEW (object));
    priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (object);

    switch (property_id) {
        case PROP_NUM_POINTS:
            {
                guint n_points = g_value_get_uint (value);

                if (n_points < 1) {
                    g_warning ("Not enough points");
                    return;
                }

                priv->points = g_realloc_n (priv->points, n_points, sizeof (gint));
                priv->n_points = n_points;
            }
            break;

        case PROP_MIN:
            {
                gint min = g_value_get_int (value); 
                
                if (min > priv->max)
                    g_warning ("%i < %i is not possible", min, priv->max);
                else {
                    priv->min = min; 
                    clamp_points (priv);
                }
            }
            break;

        case PROP_MAX:
            {
                gint max = g_value_get_int (value); 
                
                if (max < priv->min)
                    g_warning ("%i < %i is not possible", priv->min, max);
                else {
                    priv->max = max; 
                    clamp_points (priv);
                }
            }
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            return;
    }
}

static void
egg_piecewise_linear_view_get_property (GObject    *object,
                                        guint       property_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
    EggPiecewiseLinearViewPrivate *priv;

    g_return_if_fail (EGG_IS_PIECEWISE_LINEAR_VIEW (object));
    priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (object);

    switch (property_id) {
        case PROP_NUM_POINTS:
            g_value_set_uint (value, priv->n_points);
            break;
        case PROP_MIN:
            g_value_set_int (value, priv->min);
            break;
        case PROP_MAX:
            g_value_set_int (value, priv->max);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            return;
    }
}

static void
egg_piecewise_linear_view_class_init (EggPiecewiseLinearViewClass *klass)
{
    GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass  *widget_class  = GTK_WIDGET_CLASS (klass);

    gobject_class->set_property = egg_piecewise_linear_view_set_property;
    gobject_class->get_property = egg_piecewise_linear_view_get_property;

    widget_class->size_request = egg_piecewise_linear_view_size_request;
    widget_class->expose_event = egg_piecewise_linear_view_expose;
    widget_class->button_press_event = egg_piecewise_linear_button_press;
    widget_class->button_release_event = egg_piecewise_linear_button_release;
    widget_class->motion_notify_event = egg_piecewise_linear_motion_notify;

    egg_piecewise_linear_view_properties[PROP_NUM_POINTS] = 
        g_param_spec_uint("num-points",
                          "Number of data points",
                          "Number of data points",
                          1, G_MAXUINT, 2,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    egg_piecewise_linear_view_properties[PROP_MIN] = 
        g_param_spec_int("min",
                          "Minimum of the range",
                          "Minimum of the range",
                          -G_MAXINT, G_MAXINT, 0,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    egg_piecewise_linear_view_properties[PROP_MAX] = 
        g_param_spec_int("max",
                          "Maximum of the range",
                          "Maximum of the range",
                          -G_MAXINT, G_MAXINT, 1,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class, 
                                       N_PROPERTIES,
                                       egg_piecewise_linear_view_properties);

    egg_piecewise_linear_view_signals[POINT_CHANGED] =
        g_signal_new ("point-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE,
                      1, G_TYPE_UINT);

    g_type_class_add_private (klass, sizeof (EggPiecewiseLinearViewPrivate));
}

static void
egg_piecewise_linear_view_init (EggPiecewiseLinearView *view)
{
    EggPiecewiseLinearViewPrivate *priv;

    view->priv = priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (view);
    priv->border_width = 2;
    priv->n_points  = 0;
    priv->points    = NULL;
    priv->grabbed   = FALSE;
    priv->dragged   = 0;
    priv->min       = 0;
    priv->max       = 1;

    gtk_widget_add_events (GTK_WIDGET (view),
                           GDK_BUTTON_PRESS_MASK   |
                           GDK_BUTTON_RELEASE_MASK |
                           GDK_BUTTON1_MOTION_MASK |
                           GDK_POINTER_MOTION_MASK);
}
