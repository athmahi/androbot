package edu.pdx.rsurya07.accelerometer;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.Queue;

/**
 * @author Surya Ravikumar
 *
 * Activity that displays all the images found in external storage in a RecyclerView and
 * starts an new activity when one of the images in the RecyclerView is clicked on
 */
public class GalleryActivity extends AppCompatActivity implements onImageClickListener{

    private RecyclerView mRecyclerView;
    private ArrayList<Bitmap> images;
    private ArrayList<String> imageNames;

    private RecyclerView.LayoutManager mLayoutManager;
    private RecyclerView.Adapter mAdapter;

    /**
     * Method that reads all the images in the specified external storage folder into arrays
     */
    private void readFiles()
    {
        //get permission to read from external storage if permission not given
        if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {

            requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, 1);
        }

        //get root directory
        String root = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).toString()+ "/AndroBot";

        File myDir = new File(root);
        myDir.mkdirs();

        //get all files
        Queue<File> files = new LinkedList<>();
        files.addAll(Arrays.asList(myDir.listFiles()));

        //bitmap and string arrays
        images = new ArrayList<>();
        imageNames = new ArrayList<>();

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        Bitmap bitmap;
        String s;

        //read image one by one
        for(int i = 0; i < files.size(); i++)
        {
            s = ((LinkedList<File>) files).get(i).toString();   //filename

            bitmap = BitmapFactory.decodeFile(s, options);      //read file as image

            //edit filename string so the path is removed and only the filename remains
            s = s.replace(root+"/", "");
            s = s.replace(".png", "");

            //add image and name to arrays
            imageNames.add(s);
            images.add(bitmap);
        }
    }

    /**
     * Method that retrieves and initializes widgets and view
     * @param savedInstanceState
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gallery);

        readFiles();

        //get view
        mRecyclerView = findViewById(R.id.recyclerView);
        mLayoutManager = new GridLayoutManager(this, 3);    //a grid layout manager with 3 items per row
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setLayoutManager(mLayoutManager);

        mAdapter = new RecyclerAdapter(this);
        mRecyclerView.setAdapter(mAdapter);
    }

    /**
     * When an image is clicked, start a new activity after passing the image clicked on as extra
     * @param position  index of image clicked on
     */
    @Override
    public void onImageClick(int position) {

        //get the image at index
        Bitmap b = images.get(position);

        //convert it to a byte array
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        b.compress(Bitmap.CompressFormat.PNG, 100, stream);
        byte[] bytes = stream.toByteArray();

        //pass image as extra and start activity
        Intent intent = new Intent(GalleryActivity.this, PhotoActivity.class);
        intent.putExtra("IMAGE", bytes);
        startActivity(intent);

    }

    /**
     * Class that defines the adapter
     */
    private class RecyclerAdapter extends RecyclerView.Adapter<ImageViewHolder>
    {

        private onImageClickListener mOnImageClickListener;

        private RecyclerAdapter(onImageClickListener onImageClickListen)
        {
            this.mOnImageClickListener = onImageClickListen;
        }

        @NonNull
        @Override
        public ImageViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {

            //inflate the view
            View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.photo_layout, viewGroup, false);
            ImageViewHolder imageViewHolder = new ImageViewHolder(view, mOnImageClickListener);

            return imageViewHolder;
        }

        @Override
        public void onBindViewHolder(@NonNull ImageViewHolder viewHolder, int i) {

            //set image and name using index
            Bitmap bitmap = images.get(i);
            String title = imageNames.get(i);

            viewHolder.image.setImageBitmap(bitmap);
            viewHolder.name.setText(title);

        }

        @Override
        public int getItemCount() {
            //return size of array
            return imageNames.size();
        }
    }

    /**
     * view holder class
     */
    private class ImageViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener
    {
        private ImageView image;
        private TextView name;
        onImageClickListener mOnImageClickListener;

        private ImageViewHolder(@NonNull View itemView, onImageClickListener onImageClickListen) {
            super(itemView);

            this.mOnImageClickListener = onImageClickListen;

            itemView.setOnClickListener(this);
            image = itemView.findViewById(R.id.photoView);
            name = itemView.findViewById(R.id.imageName);
        }


        @Override
        public void onClick(View view) {
            mOnImageClickListener.onImageClick(getAdapterPosition());
        }
    }

}
