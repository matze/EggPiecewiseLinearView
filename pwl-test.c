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

#include <gtk/gtk.h>
#include "egg-data-points.h"
#include "egg-piecewise-linear-view.h"

static gboolean
on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    return FALSE;
}

static void
on_point_changed (EggPiecewiseLinearView *view, guint index)
{
    EggDataPoints *points;

    points = egg_piecewise_linear_view_get_points (view);
    g_print ("point set to (%f, %f)\n",
             egg_data_points_get_x_value (points, index),
             egg_data_points_get_y_value (points, index));
}

int
main (int argc, char* argv[])
{
    GtkWidget *window;
    GtkWidget *container;
    GtkWidget *view;
    GtkWidget *fixed_button_box;
    GtkWidget *fixed_x_button;
    GtkWidget *fixed_y_button;
    GtkWidget *fixed_borders_button;
    GtkWidget *grid_button_box;
    GtkWidget *grid_x_button;
    GtkWidget *grid_y_button;
    GtkWidget *grid_x_enable_button;
    GtkWidget *grid_y_enable_button;
    GtkAdjustment *grid_x;
    GtkAdjustment *grid_y;
    EggDataPoints *points;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    g_signal_connect (window, "delete-event", G_CALLBACK (on_delete_event), NULL);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    /* Add some points */
    points = egg_data_points_new (0.0, 2.0, 0.0, 300.0);
    egg_data_points_add_point (points, 0.0, 0.0, 1.0);
    egg_data_points_add_point (points, 0.5, 100.0, 1.0);
    egg_data_points_add_point (points, 1.5, 100.0, 1.0);
    egg_data_points_add_point (points, 2.0, 200.0, 1.0);

    view = egg_piecewise_linear_view_new ();
    egg_piecewise_linear_view_set_points (EGG_PIECEWISE_LINEAR_VIEW (view), points);

    g_signal_connect (view, "point-changed", G_CALLBACK (on_point_changed), NULL);

#if GTK_CHECK_VERSION(3, 2, 0)
    container = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
#else
    container = gtk_vbox_new (FALSE, 6);
#endif

    /* Create fix buttons */
#if GTK_CHECK_VERSION(3, 2, 0)
    fixed_button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    grid_button_box  = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
#else
    fixed_button_box = gtk_hbutton_box_new ();
    grid_button_box  = gtk_hbutton_box_new ();
#endif

    fixed_x_button = gtk_check_button_new_with_label ("Fixed X");
    fixed_y_button = gtk_check_button_new_with_label ("Fixed Y");
    fixed_borders_button = gtk_check_button_new_with_label ("Fixed Borders");

    /* Create grid buttons */
    grid_x_enable_button = gtk_check_button_new_with_label ("Snap to X");
    grid_y_enable_button = gtk_check_button_new_with_label ("Snap to Y");
    grid_x = GTK_ADJUSTMENT (gtk_adjustment_new (0.25, 0.1, 3.0, 0.01, 1.0, 0.0));
    grid_y = GTK_ADJUSTMENT (gtk_adjustment_new (10.0, 1.0, 300.0, 1.0, 10.0, 0.0));

    grid_x_button = gtk_spin_button_new (grid_x, 0.1, 3);
    grid_y_button = gtk_spin_button_new (grid_y, 1.0, 3);

    /* Layout the widgets */
    gtk_box_pack_start (GTK_BOX (fixed_button_box), fixed_x_button, TRUE, TRUE, 3);
    gtk_box_pack_start (GTK_BOX (fixed_button_box), fixed_y_button, TRUE, TRUE, 3);
    gtk_box_pack_start (GTK_BOX (fixed_button_box), fixed_borders_button, TRUE, TRUE, 3);

    gtk_box_pack_start (GTK_BOX (grid_button_box), grid_x_enable_button, TRUE, TRUE, 3);
    gtk_box_pack_start (GTK_BOX (grid_button_box), grid_x_button, TRUE, TRUE, 3);
    gtk_box_pack_start (GTK_BOX (grid_button_box), grid_y_enable_button, TRUE, TRUE, 3);
    gtk_box_pack_start (GTK_BOX (grid_button_box), grid_y_button, TRUE, TRUE, 3);

    gtk_box_pack_start (GTK_BOX (container), view, TRUE, TRUE, 6);
    gtk_box_pack_start (GTK_BOX (container), fixed_button_box, FALSE, TRUE, 6);
    gtk_box_pack_start (GTK_BOX (container), grid_button_box, FALSE, TRUE, 6);

    gtk_container_add (GTK_CONTAINER (window), container);

    /* Connect widgets with properties */
    g_object_bind_property (fixed_x_button, "active", view, "fixed-x-axis", G_BINDING_BIDIRECTIONAL);
    g_object_bind_property (fixed_y_button, "active", view, "fixed-y-axis", G_BINDING_BIDIRECTIONAL);
    g_object_bind_property (fixed_borders_button, "active", view, "fixed-borders", G_BINDING_BIDIRECTIONAL);

    g_object_bind_property (grid_x_enable_button, "active", view, "snap-to-x", G_BINDING_BIDIRECTIONAL);
    g_object_bind_property (grid_y_enable_button, "active", view, "snap-to-y", G_BINDING_BIDIRECTIONAL);
    g_object_bind_property (grid_x, "value", view, "x-grid-increment", G_BINDING_BIDIRECTIONAL);
    g_object_bind_property (grid_y, "value", view, "y-grid-increment", G_BINDING_BIDIRECTIONAL);

    /* Initialize the view with something useful */
    egg_piecewise_linear_view_set_grid (EGG_PIECEWISE_LINEAR_VIEW (view), 0.25, 50.0);

    gtk_widget_show_all (window);
    gtk_main ();

    return 0;
}
