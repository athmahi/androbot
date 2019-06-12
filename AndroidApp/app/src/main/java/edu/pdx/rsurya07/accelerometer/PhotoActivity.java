package edu.pdx.rsurya07.accelerometer;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.ImageView;

/**
 * @author Surya Ravikumar
 *
 * Activty that recives an image as byte array and displaeys it
 */
public class PhotoActivity extends AppCompatActivity {

    private ImageView mImageView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_photo);

        //get extra and convert it to bitmap
        byte[] bytes = getIntent().getByteArrayExtra("IMAGE");
        Bitmap b = BitmapFactory.decodeByteArray(bytes, 0, bytes.length);

        //display
        mImageView = findViewById(R.id.imageShow);
        mImageView.setImageBitmap(b);
    }
}
