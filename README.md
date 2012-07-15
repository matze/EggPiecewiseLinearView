# EggPiecewiseLinearView

A generic view to show data points that approximate a piecewise linear function,
inspired by the GIMP curves tool. Data is managed by the `EggDataPoints` store:

    EggDataPoints *points = egg_data_points_new (0.0, 5.0, 0.0, 5.0);
    guint index = egg_data_points_add_point (points, 1.0, 1.0, 0.2);

    EggPiecewiseLinearView *view = egg_piecewise_linear_view_new ();
    egg_piecewise_linear_view_set_points (points);
    
    g_signal_connect (view, "point-changed", G_CALLBACK (on_point_changed), NULL);

Through the index, the underlying `GtkAdjustment` can be extracted to monitor
coordinate changes:

    GtkAdjustment *adj = egg_data_points_get_x (points, index);
    g_signal_connect (adj, "value-changed", G_CALLBACK (on_value_change), NULL);

The view also features:

* fixing axes
* adjustable grids
* snap-to-grid
