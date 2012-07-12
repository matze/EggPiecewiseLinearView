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
#include "egg-piecewise-linear-view.h"

static gboolean
on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    return FALSE;
}

int
main (int argc, char* argv[])
{
    GtkWidget *window; 
    GtkWidget *view;
    EggPiecewiseLinearView *pwl;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    g_signal_connect (window, "delete-event", G_CALLBACK (on_delete_event), NULL);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    view = egg_piecewise_linear_view_new (3, 0, 300);
    pwl = EGG_PIECEWISE_LINEAR_VIEW (view);
    egg_piecewise_linear_view_set_point (pwl, 0, 0);
    egg_piecewise_linear_view_set_point (pwl, 1, 100);
    egg_piecewise_linear_view_set_point (pwl, 2, 200);

    gtk_container_add (GTK_CONTAINER (window), view);
    gtk_widget_show_all (window);
    gtk_main ();

    return 0;
}
