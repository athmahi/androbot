package edu.pdx.rsurya07.accelerometer;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.View;

import java.util.ArrayList;

/**
 * @author Surya Ravikumar
 *
 * Custom class that extends the View class. Includes methods to draw the path on screen
 */
public class LineView extends View {


    private Paint paint = new Paint();  //draw properties
    private ArrayList<PointF> pointA;   //array to hold coordinates

    public LineView(Context context)
    {
        super(context);
    }

    public LineView(Context context, @Nullable AttributeSet attrs)
    {
        super(context, attrs);
    }

    public LineView(Context context, @Nullable AttributeSet attrs, int defStyleAttr)
    {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void onDraw(Canvas canvas)
    {
        paint.setColor(Color.RED);  //set draw color to red

        paint.setStrokeWidth(10);   //set draw width

        //draw lines between all the coordinates in the array
        for(int i = 0; i < pointA.size()-1; i++)
            canvas.drawLine(pointA.get(i).x, pointA.get(i).y, pointA.get(i+1).x, pointA.get(i+1).y, paint);

        //draw circle to show starting points
        canvas.drawCircle(pointA.get(0).x, pointA.get(0).y, 15, paint);

        //change color to blue
        paint.setColor(Color.BLUE);

        //draw circle to denote current location of bot in blue
        if(pointA.size() > 1)
            canvas.drawCircle(pointA.get(pointA.size()-1).x, pointA.get(pointA.size()-1).y, 15, paint);

        super.onDraw(canvas);
    }

    /**
     * Method that sets the private variable with passed in coordinates
     * @param point array that contains all coordinates
     */
    public void setPointA(ArrayList<PointF> point)
    {
        pointA = point ;
    }

    /**
     * Method that invalidates the current layout and requests a new layout
     */
    public void draw()
    {
        invalidate();
        requestLayout();
    }

}
