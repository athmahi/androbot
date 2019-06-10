package edu.pdx.rsurya07.accelerometer;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.PointF;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaScannerConnection;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewTreeObserver;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.FirebaseApp;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import java.io.File;
import java.io.FileOutputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;


public class MainActivity extends AppCompatActivity implements SensorEventListener {

    public static int MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE = 1;
    private TextView direction, Xcor, Ycor, Timestamp;
    private int lineViewWidth, lineViewHeight, hBias, wBias;
    private LinearLayout toSave;
    private Switch onOff, autoMode;
    private Sensor accelerometer;
    private SensorManager sensor;
    private DatabaseReference mDatabase;
    private ArrayList<PointF> mPointA = new ArrayList<>();
    private LineView mLineView;
    private Button saveButton;
    private Button viewGallery;
    private ImageView mImageView;
    private Bitmap image;
    private boolean first;


    private void init()
    {
        mDatabase.child("botOn").setValue(1);
        mDatabase.child("autoMode").setValue(1);
        mDatabase.child("X").setValue(500);
        mDatabase.child("Y").setValue(500);

        Log.i("H: ", String.valueOf(lineViewHeight));
        Log.i("W: ", String.valueOf(lineViewWidth));

        mPointA.add(new PointF(lineViewWidth/2, lineViewHeight/2));
        mPointA.add(new PointF(lineViewWidth/2, lineViewHeight/2));
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("MM/dd/yyyy   hh:mm");
        String format = simpleDateFormat.format(new Date());

        Xcor.setText("500");
        Ycor.setText("500");
        Timestamp.setText(format);

        mLineView.setPointA(mPointA);
        mLineView.draw();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        FirebaseApp.initializeApp(this);
        mDatabase = FirebaseDatabase.getInstance().getReference();

        sensor = (SensorManager) getSystemService(SENSOR_SERVICE);
        accelerometer = sensor.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        sensor.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);

        direction = findViewById(R.id.direction);
        Xcor = findViewById(R.id.X);
        Ycor = findViewById(R.id.Y);
        mLineView = findViewById(R.id.lineView);

        first  = true;

        ViewTreeObserver vto = mLineView.getViewTreeObserver();
        vto.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
            public boolean onPreDraw() {
                lineViewHeight = mLineView.getMeasuredHeight();
                lineViewWidth = mLineView.getMeasuredWidth();
                hBias = (lineViewHeight - 1000)/2;
                wBias = (lineViewWidth - 1000)/2;

                if(first)
                {
                    first = false;
                    init();
                    getDataInit();
                }
                return true;
            }
        });

        Timestamp = findViewById(R.id.Timestamp);
        toSave = findViewById(R.id.toSave);

        onOff = findViewById(R.id.onOff);
        autoMode = findViewById(R.id.autoMode);

        mImageView = findViewById(R.id.imageView);

        saveButton = findViewById(R.id.saveButton);
        saveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                toSave.destroyDrawingCache();
                toSave.buildDrawingCache(true);
                image = toSave.getDrawingCache(true);

                mImageView.setImageBitmap(image);

                if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {

                    if (shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                        // Explain to the user why we need to read the contacts
                    }

                    requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE);
                }

                String root = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).toString()+ "/ece558";

                Log.d("My app", root);
                File myDir = new File(root);
                myDir.mkdirs();

                SimpleDateFormat sdf = new SimpleDateFormat("MMddyyyy_hh-mm-ss");
                String currentDateandTime = sdf.format(new Date());
                String fname = "Path-" + currentDateandTime + ".png";

                File file = new File(myDir, fname);

                if (file.exists()) file.delete();
                Log.i("LOAD", root + fname);

                try {
                    FileOutputStream out = new FileOutputStream(file);
                    image.compress(Bitmap.CompressFormat.PNG, 50, out);
                    out.flush();
                    out.close();
                }
                catch (Exception e) {
                    e.printStackTrace();
                }

                MediaScannerConnection.scanFile(MainActivity.this, new String[]{file.getPath()}, new String[]{"image/jpeg"}, null);

                Toast.makeText(MainActivity.this, "Path saved!", Toast.LENGTH_LONG).show();
            }
        });

        viewGallery = findViewById(R.id.viewGallery);
        viewGallery.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(MainActivity.this, GalleryActivity.class);
                startActivity(intent);
            }
        });

        onOff.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if(b)
                    mDatabase.child("botOn").setValue(2);
                else
                    mDatabase.child("botOn").setValue(1);
            }
        });

        autoMode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if(b)
                    mDatabase.child("autoMode").setValue(2);

                else
                    mDatabase.child("autoMode").setValue(1);
            }
        });
    }

    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {

        if(sensorEvent.values[0] > 5 && (sensorEvent.values[1] < 2 && sensorEvent.values[1] > -2))
        {
            direction.setText("D: L");
            mDatabase.child("directionfromApp").setValue(5);
        }

        else if(sensorEvent.values[0] < -5 && (sensorEvent.values[1] < 2 && sensorEvent.values[1] > -2))
        {
            direction.setText("D: R");
            mDatabase.child("directionfromApp").setValue(4);
        }

        else if(sensorEvent.values[1] > 5 && (sensorEvent.values[0] < 2 && sensorEvent.values[0] > -2))
        {
            direction.setText("D: B");
            mDatabase.child("directionfromApp").setValue(3);
        }

        else if(sensorEvent.values[1] < -3 && (sensorEvent.values[0] < 2 && sensorEvent.values[0] > -2))
        {
            direction.setText("D: F");
            mDatabase.child("directionfromApp").setValue(2);
        }

        else
        {
            direction.setText("D: S");
            mDatabase.child("directionfromApp").setValue(1);
        }

    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }

    private void getDataInit() {
        ValueEventListener dataListener = new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                Long X = (Long) dataSnapshot.child("X").getValue();
                Long Y = (Long) dataSnapshot.child("Y").getValue();

                int pointSet = ((Long) dataSnapshot.child("pointSet").getValue()).intValue();

                if(pointSet == 1)
                {
                    Long h = Y + hBias;
                    Long w = X + wBias;

                    if(h < 0)
                        h = new Long(0);

                    if(w < 0)
                        w = new Long(0);

                    mPointA.add(new PointF(w.intValue(), h.intValue()));
                    mLineView.setPointA(mPointA);
                    mLineView.draw();

                    Xcor.setText(String.valueOf(X));
                    Ycor.setText(String.valueOf(Y));

                    SimpleDateFormat simpleDateFormat = new SimpleDateFormat("MM/dd/yyyy   hh:mm");
                    String format = simpleDateFormat.format(new Date());

                    Timestamp.setText(format);

                    mDatabase.child("pointSet").setValue(0);
                }
            }

            @Override
            public void onCancelled(DatabaseError databaseError) {
                Log.w("ACC", "onCancelled", databaseError.toException());
            }
        };

        mDatabase.addValueEventListener(dataListener);
    }
}
