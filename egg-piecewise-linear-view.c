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
#include "egg-data-points.h"

G_DEFINE_TYPE (EggPiecewiseLinearView, egg_piecewise_linear_view, GTK_TYPE_DRAWING_AREA)

#define EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), EGG_TYPE_PIECEWISE_LINEAR_VIEW, EggPiecewiseLinearViewPrivate))

#define MIN_WIDTH   128
#define MIN_HEIGHT  128

struct _EggPiecewiseLinearViewPrivate
{
    gint            border_width;
    EggDataPoints  *points;
    GdkCursorType   cursor_type;

    gboolean        grabbed;
    guint           dragged_index;
    GtkAdjustment  *dragged_x;
    GtkAdjustment  *dragged_y;

    gboolean        fixed_x;
    gboolean        fixed_y;
    gboolean        fixed_borders;
    gboolean        grid_x;
    gboolean        grid_y;
    gdouble         grid_x_increment;
    gdouble         grid_y_increment;
};

enum
{
    PROP_0,
    PROP_GRID_X,
    PROP_GRID_Y,
    PROP_GRID_X_INCREMENT,
    PROP_GRID_Y_INCREMENT,
    PROP_FIXED_X,
    PROP_FIXED_Y,
    PROP_FIXED_BORDERS,
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
egg_piecewise_linear_view_new (void)
{
    return GTK_WIDGET (g_object_new (EGG_TYPE_PIECEWISE_LINEAR_VIEW, NULL));
}

void
egg_piecewise_linear_view_set_points (EggPiecewiseLinearView *view, EggDataPoints *points)
{
    g_return_if_fail (EGG_PIECEWISE_LINEAR_VIEW (view));

    g_object_ref (points);
    view->priv->points = points;
}

EggDataPoints *
egg_piecewise_linear_view_get_points (EggPiecewiseLinearView *view)
{
    g_return_val_if_fail (EGG_PIECEWISE_LINEAR_VIEW (view), NULL);

    return view->priv->points;
}

static void
egg_piecewise_linear_view_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
    /* TODO: maybe add border width */
    requisition->width = MIN_WIDTH;
    requisition->height = MIN_HEIGHT;
}

static void
map_x_to_window (gdouble *x, gdouble xscale, gdouble width)
{
    *x /= xscale;
    *x *= width;
}

static void
map_y_to_window (gdouble *y, gdouble yscale, gdouble height)
{
    *y /= yscale;
    *y = height - (*y) * height;
}

static gboolean
egg_piecewise_linear_view_expose (GtkWidget *widget, GdkEventExpose *event)
{
    const static gdouble dashes[2] = { 0.5, 4.0 };

    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (widget);
    GtkStyle        *style = gtk_widget_get_style (widget);
    GtkAllocation    allocation;
    cairo_t         *cr;
    gint             border;
    gint             width, height;
    gdouble          x, y;
    gdouble          lower_x, upper_x;
    gdouble          lower_y, upper_y;
    gdouble          xscale, yscale;
    guint            n_points;

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

    /* Scale to the data point coordinate system */
    egg_data_points_get_x_range (priv->points, &lower_x, &upper_x);
    egg_data_points_get_y_range (priv->points, &lower_y, &upper_y);

    xscale = upper_x - lower_x;
    yscale = upper_y - lower_y;

    /* Draw grid */
    if (priv->grid_x) {
        for (gdouble x = lower_x + priv->grid_x_increment; x < upper_x; x += priv->grid_x_increment) {
            gdouble xp = x;
            map_x_to_window (&xp, xscale, width);
            xp += border;
            cairo_move_to (cr, floor (xp), border);             
            cairo_line_to (cr, floor (xp), height);
        } 
    }

    if (priv->grid_y) {
        for (gdouble y = lower_y + priv->grid_y_increment; y < upper_y; y += priv->grid_y_increment) {
            gdouble yp = y;
            map_y_to_window (&yp, yscale, height);
            yp += border;
            cairo_move_to (cr, border, floor (yp));             
            cairo_line_to (cr, width, floor (yp));
        } 
    }

    cairo_set_dash (cr, dashes, 2, 0.0);
    cairo_stroke (cr);

    /* Draw lines */
    n_points = egg_data_points_get_num (priv->points);
    x = egg_data_points_get_x_value (priv->points, 0);
    y = egg_data_points_get_y_value (priv->points, 0);

    cairo_set_line_width (cr, 1.5);
    map_x_to_window (&x, xscale, width);
    map_y_to_window (&y, yscale, height);
    cairo_move_to (cr, x + border, y + border);

    for (guint i = 1; i < n_points; i++) {
        x = egg_data_points_get_x_value (priv->points, i);
        y = egg_data_points_get_y_value (priv->points, i);
        map_x_to_window (&x, xscale, width);
        map_y_to_window (&y, yscale, height);
        cairo_line_to (cr, x + border, y + border);
    }

    cairo_set_dash (cr, NULL, 0, 0.0);
    cairo_stroke (cr);

    /* Draw points */
#define RADIUS 3
    for (guint i = 0; i < n_points; i++) {
        x = egg_data_points_get_x_value (priv->points, i);
        y = egg_data_points_get_y_value (priv->points, i);
        map_x_to_window (&x, xscale, width);
        map_y_to_window (&y, yscale, height);
        cairo_move_to (cr, x + RADIUS + border, y + border);
        cairo_arc (cr, x + border, y + border, RADIUS, 0, 2 * G_PI);
        cairo_fill (cr);
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

static void
get_closest_point (GtkWidget *widget, gint in_x, gint in_y, gdouble *out_x, gdouble *out_y, guint *index, gdouble *distance)
{
    EggPiecewiseLinearView
                    *view = EGG_PIECEWISE_LINEAR_VIEW (widget);
    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (view);
    GtkAllocation    allocation;
    gint             border;
    gint             width, height;
    gdouble          lower_x, upper_x;
    gdouble          lower_y, upper_y;
    gdouble          x, y;

    gtk_widget_get_allocation (widget, &allocation);
    border = priv->border_width;
    width  = allocation.width - 2 * border;
    height = allocation.height - 2 * border;
    x = (gdouble) (in_x - border) / (gdouble) width;
    y = (gdouble) (height - in_y - border) / (gdouble) height;

    egg_data_points_get_x_range (priv->points, &lower_x, &upper_x);
    egg_data_points_get_y_range (priv->points, &lower_y, &upper_y);

    x *= upper_x;
    y *= upper_y;

    *index = egg_data_get_closest_point (priv->points, x, y, distance);
    *out_x = x;
    *out_y = y;
}

static gboolean
egg_piecewise_linear_button_press (GtkWidget *widget, GdkEventButton *event)
{
    EggPiecewiseLinearView
                    *view = EGG_PIECEWISE_LINEAR_VIEW (widget);
    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (view);
    gdouble          x, y;
    gdouble          distance;
    guint            closest;
    guint            n_points;

    if (event->button != 1)
        return TRUE;

    get_closest_point (widget, event->x, event->y, &x, &y, &closest, &distance);
    n_points = egg_data_points_get_num (priv->points);

    if (!priv->fixed_borders || (closest > 0 && (closest < n_points - 1))) {
        if (distance < 0.1) {
            priv->grabbed   = TRUE;
            priv->dragged_x = egg_data_points_get_x (priv->points, closest);
            priv->dragged_y = egg_data_points_get_y (priv->points, closest);
            priv->dragged_index = closest;

            set_cursor_type (view, GDK_FLEUR);
        }
    }

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

    if (priv->grabbed) {
        g_signal_emit (view, 
                       egg_piecewise_linear_view_signals[POINT_CHANGED], 
                       0, priv->dragged_index);
    }

    priv->grabbed = FALSE;
    set_cursor_type (view, GDK_TCROSS);
    return TRUE;
}

static gboolean
egg_piecewise_linear_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
    EggPiecewiseLinearViewPrivate
                    *priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (widget);
    GdkCursorType    cursor_type = GDK_TCROSS;
    gdouble          x, y;
    guint            n_points;
    gdouble          distance;
    guint            closest;

    get_closest_point (widget, event->x, event->y, &x, &y, &closest, &distance);
    n_points = egg_data_points_get_num (priv->points);

    if (!priv->grabbed) {
        if (!priv->fixed_borders || (closest > 0 && closest < n_points - 1)) {
            if (distance < 0.1)
                cursor_type = GDK_FLEUR;
        }
    }
    else {
        if (!priv->fixed_x)
            gtk_adjustment_set_value (priv->dragged_x, x);

        if (!priv->fixed_y)
            gtk_adjustment_set_value (priv->dragged_y, y);

        cursor_type = GDK_FLEUR;
        gtk_widget_queue_draw (widget);
    }

    set_cursor_type (EGG_PIECEWISE_LINEAR_VIEW (widget), cursor_type);
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
        case PROP_GRID_X:
            priv->grid_x = g_value_get_boolean (value);
            break;
        case PROP_GRID_Y:
            priv->grid_y = g_value_get_boolean (value);
            break;
        case PROP_FIXED_X:
            priv->fixed_x = g_value_get_boolean (value);
            break;
        case PROP_FIXED_Y:
            priv->fixed_y = g_value_get_boolean (value);
            break;
        case PROP_FIXED_BORDERS:
            priv->fixed_borders = g_value_get_boolean (value);
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
        case PROP_GRID_X:
            g_value_set_boolean (value, priv->grid_x);
            break;
        case PROP_GRID_Y:
            g_value_set_boolean (value, priv->grid_y);
            break;
        case PROP_FIXED_X:
            g_value_set_boolean (value, priv->fixed_x);
            break;
        case PROP_FIXED_Y:
            g_value_set_boolean (value, priv->fixed_y);
            break;
        case PROP_FIXED_BORDERS:
            g_value_set_boolean (value, priv->fixed_borders);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            return;
    }
}

static void
egg_piecewise_linear_view_dispose (GObject *object)
{
    EggPiecewiseLinearViewPrivate *priv;

    priv = EGG_PIECEWISE_LINEAR_VIEW_GET_PRIVATE (object);
    g_object_unref (priv->points);

    G_OBJECT_CLASS (egg_piecewise_linear_view_parent_class)->dispose (object);
}

static void
egg_piecewise_linear_view_class_init (EggPiecewiseLinearViewClass *klass)
{
    GObjectClass    *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass  *widget_class  = GTK_WIDGET_CLASS (klass);

    gobject_class->set_property = egg_piecewise_linear_view_set_property;
    gobject_class->get_property = egg_piecewise_linear_view_get_property;
    gobject_class->dispose = egg_piecewise_linear_view_dispose;

    widget_class->size_request = egg_piecewise_linear_view_size_request;
    widget_class->expose_event = egg_piecewise_linear_view_expose;
    widget_class->button_press_event = egg_piecewise_linear_button_press;
    widget_class->button_release_event = egg_piecewise_linear_button_release;
    widget_class->motion_notify_event = egg_piecewise_linear_motion_notify;

    egg_piecewise_linear_view_properties[PROP_GRID_X] =
        g_param_spec_boolean ("show-x-grid",
                              "TRUE if grid on x-axis should be shown",
                              "TRUE if grid on x-axis should be shown",
                              FALSE,
                              G_PARAM_READWRITE);

    egg_piecewise_linear_view_properties[PROP_GRID_Y] =
        g_param_spec_boolean ("show-y-grid",
                              "TRUE if grid on y-axis should be shown",
                              "TRUE if grid on y-axis should be shown",
                              FALSE,
                              G_PARAM_READWRITE);

    egg_piecewise_linear_view_properties[PROP_GRID_X_INCREMENT] =
        g_param_spec_double ("x-grid-increment",
                             "Number of values to skip between x-axis grid lines",
                             "Number of values to skip between x-axis grid lines",
                             0.0, DBL_MAX, 1.0,
                             G_PARAM_READWRITE);

    egg_piecewise_linear_view_properties[PROP_GRID_Y_INCREMENT] =
        g_param_spec_double ("y-grid-increment",
                             "Number of values to skip between y-axis grid lines",
                             "Number of values to skip between y-axis grid lines",
                             0.0, DBL_MAX, 1.0,
                             G_PARAM_READWRITE);

    egg_piecewise_linear_view_properties[PROP_FIXED_X] =
        g_param_spec_boolean ("fixed-x",
                              "TRUE if x values cannot be changed",
                              "TRUE if x values cannot be changed",
                              FALSE,
                              G_PARAM_READWRITE);

    egg_piecewise_linear_view_properties[PROP_FIXED_Y] =
        g_param_spec_boolean ("fixed-y",
                              "TRUE if y values cannot be changed",
                              "TRUE if y values cannot be changed",
                              FALSE,
                              G_PARAM_READWRITE);

    egg_piecewise_linear_view_properties[PROP_FIXED_BORDERS] =
        g_param_spec_boolean ("fixed-borders",
                              "TRUE if border values cannot be changed",
                              "TRUE if border values cannot be changed",
                              FALSE,
                              G_PARAM_READWRITE);

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
    priv->points    = NULL;
    priv->grabbed   = FALSE;
    priv->grid_x    = TRUE;
    priv->grid_y    = TRUE;
    priv->fixed_x   = FALSE;
    priv->fixed_y   = FALSE;
    priv->fixed_borders = FALSE;
    priv->grid_x_increment = 1.0;
    priv->grid_y_increment = 100.0;

    gtk_widget_add_events (GTK_WIDGET (view),
                           GDK_BUTTON_PRESS_MASK   |
                           GDK_BUTTON_RELEASE_MASK |
                           GDK_BUTTON1_MOTION_MASK |
                           GDK_POINTER_MOTION_MASK);
}
