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

/**
 * @author Surya Ravikumar
 *
 * Activity that the user is greeted with. This acitivty
 *      - Displays the app title
 *      - Graphs the bot movement
 *      - Initializes widgets and sets onclicklisteners
 *      - Sets up firebase and sets data change listener
 *      - Saves path as a picture to external storage
 *      - Opens gallery activity
 */
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

    /**
     * Function that is called once at start. Sets default values on Firebase, and draw starting location on map.
     */
    private void init()
    {
        //set default values on Firebase
        mDatabase.child("botOn").setValue(1);
        mDatabase.child("autoMode").setValue(1);
        mDatabase.child("X").setValue(500);
        mDatabase.child("Y").setValue(500);

        //add 2 points to array (both points are same and are the center points in the map)
        mPointA.add(new PointF(lineViewWidth/2, lineViewHeight/2));
        mPointA.add(new PointF(lineViewWidth/2, lineViewHeight/2));

        //get current time
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("MM/dd/yyyy   hh:mm");
        String format = simpleDateFormat.format(new Date());

        //set X, Y coordinates and time on app
        Xcor.setText("500");
        Ycor.setText("500");
        Timestamp.setText(format);

        //initialize points and draw
        mLineView.setPointA(mPointA);
        mLineView.draw();
    }

    /**
     * Method that retrieves and initializes widgets. Initializes Firebase.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //initialize Firebase
        FirebaseApp.initializeApp(this);
        mDatabase = FirebaseDatabase.getInstance().getReference();

        //get accelerometer interface
        sensor = (SensorManager) getSystemService(SENSOR_SERVICE);
        accelerometer = sensor.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        sensor.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);

        //retrieve widgets

        //textviews
        direction = findViewById(R.id.direction);
        Xcor = findViewById(R.id.X);
        Ycor = findViewById(R.id.Y);

        //custom view where map is drawn
        mLineView = findViewById(R.id.lineView);

        //variable that is set when app is created to call init functions only once at start
        first  = true;

        //get observer when to see when layout has been setup and drawn
        ViewTreeObserver vto = mLineView.getViewTreeObserver();
        vto.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
            public boolean onPreDraw() {
                //get width and height of view in pixels dynamically
                //height and width vary with phone as it is based on pixel density
                lineViewHeight = mLineView.getMeasuredHeight();
                lineViewWidth = mLineView.getMeasuredWidth();

                //calculate bias so map is centered at 500, 500
                //map is constrained between 0,0 and 1000,1000
                hBias = (lineViewHeight - 1000)/2;
                wBias = (lineViewWidth - 1000)/2;

                //if going through first time
                if(first)
                {
                    first = false;  //set to false so this doesn't go through after first time

                    //call init functions
                    init();
                    getDataInit();
                }
                return true;
            }
        });

        //timestamp textview
        Timestamp = findViewById(R.id.Timestamp);

        //layout that is to be saved as image - contains timestamp and map
        toSave = findViewById(R.id.toSave);

        //switches
        onOff = findViewById(R.id.onOff);
        autoMode = findViewById(R.id.autoMode);

        //imageView that displays image last saved
        mImageView = findViewById(R.id.imageView);

        //button to save path as image
        saveButton = findViewById(R.id.saveButton);
        saveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                toSave.destroyDrawingCache();    //delete previous cache
                toSave.buildDrawingCache(true); //rebuild cache
                image = toSave.getDrawingCache(true);   //get layout as bitmap

                mImageView.setImageBitmap(image);   //set image in imageview

                //get permissions to store file if already not granted before saving to external storage
                if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {

                    requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE);
                }

                //get directory
                String root = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).toString()+ "/AndroBot";

                File myDir = new File(root);
                myDir.mkdirs();     //create directory if it doesnt exist

                //get current time to use it the create file name
                SimpleDateFormat sdf = new SimpleDateFormat("MMddyyyy_hh-mm-ss");
                String currentDateandTime = sdf.format(new Date());
                String fname = "Path-" + currentDateandTime + ".png";

                File file = new File(myDir, fname); //create file

                //delete file if it already exists
                if (file.exists())
                    file.delete();

                try {
                    //write file to storage
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

        //view gallery button
        viewGallery = findViewById(R.id.viewGallery);
        viewGallery.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //start new activity
                Intent intent = new Intent(MainActivity.this, GalleryActivity.class);
                startActivity(intent);
            }
        });

        //switch to turn on and off bot
        onOff.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {

                //write to firebase
                if(b)
                    mDatabase.child("botOn").setValue(2);   //on
                else
                    mDatabase.child("botOn").setValue(1);   //off
            }
        });

        //switch between accelerometer mode and obstacle detection mode
        autoMode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {

                //write to firebase
                if(b)
                    mDatabase.child("autoMode").setValue(2);    //obstacle detection mode

                else
                    mDatabase.child("autoMode").setValue(1);    //accelerometer mode
            }
        });
    }

    /**
     * Method that is invoked when accelerometer value changes
     * @param sensorEvent
     */
    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {

        //get forward, reverse, right, left directions based on X, Y, Z values and write it to firebase and textView

        //LEFT
        if(sensorEvent.values[0] > 5 && (sensorEvent.values[1] < 2 && sensorEvent.values[1] > -2))
        {
            direction.setText("D: L");
            mDatabase.child("directionfromApp").setValue(5);
        }

        //RIGHT
        else if(sensorEvent.values[0] < -5 && (sensorEvent.values[1] < 2 && sensorEvent.values[1] > -2))
        {
            direction.setText("D: R");
            mDatabase.child("directionfromApp").setValue(4);
        }

        //REVERSE
        else if(sensorEvent.values[1] > 5 && (sensorEvent.values[0] < 2 && sensorEvent.values[0] > -2))
        {
            direction.setText("D: B");
            mDatabase.child("directionfromApp").setValue(3);
        }

        //FORWARD
        else if(sensorEvent.values[1] < -3 && (sensorEvent.values[0] < 2 && sensorEvent.values[0] > -2))
        {
            direction.setText("D: F");
            mDatabase.child("directionfromApp").setValue(2);
        }

        //STOPPED
        else
        {
            direction.setText("D: S");
            mDatabase.child("directionfromApp").setValue(1);
        }

    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }

    /**
     * Method that is called to read firebase once and sets data change listener
     */
    private void getDataInit() {
        ValueEventListener dataListener = new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {

                //read X and Y coordinates
                Long X = (Long) dataSnapshot.child("X").getValue();
                Long Y = (Long) dataSnapshot.child("Y").getValue();

                //read point set variable - used to finalize the coordinates
                int pointSet = ((Long) dataSnapshot.child("pointSet").getValue()).intValue();

                //if the X and Y coordinates are final
                if(pointSet == 1)
                {
                    //add the bias
                    Long h = Y + hBias;
                    Long w = X + wBias;

                    //limit lower bound to 0
                    if(h < 0)
                        h = new Long(0);

                    if(w < 0)
                        w = new Long(0);

                    //add point to array and draw the map
                    mPointA.add(new PointF(w.intValue(), h.intValue()));
                    mLineView.setPointA(mPointA);
                    mLineView.draw();

                    //set text views with coordinates
                    Xcor.setText(String.valueOf(X));
                    Ycor.setText(String.valueOf(Y));

                    //get current time and set text view
                    SimpleDateFormat simpleDateFormat = new SimpleDateFormat("MM/dd/yyyy   hh:mm");
                    String format = simpleDateFormat.format(new Date());

                    Timestamp.setText(format);

                    //reset point set variable
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
