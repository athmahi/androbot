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

public class LineView extends View {
    private Paint paint = new Paint();

    private ArrayList<PointF> pointA;

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
        paint.setColor(Color.RED);

        paint.setStrokeWidth(10);


        for(int i = 0; i < pointA.size()-1; i++)
            canvas.drawLine(pointA.get(i).x, pointA.get(i).y, pointA.get(i+1).x, pointA.get(i+1).y, paint);

        canvas.drawCircle(pointA.get(0).x, pointA.get(0).y, 15, paint);

        paint.setColor(Color.BLUE);

        if(pointA.size() > 1)
            canvas.drawCircle(pointA.get(pointA.size()-1).x, pointA.get(pointA.size()-1).y, 15, paint);

        super.onDraw(canvas);
    }

    public void setPointA(ArrayList<PointF> point)
    {
        pointA = point ;
    }

    public void draw()
    {
        invalidate();
        requestLayout();
    }

}
