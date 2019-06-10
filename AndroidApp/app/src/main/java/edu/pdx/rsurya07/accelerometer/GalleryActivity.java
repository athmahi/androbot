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

public class GalleryActivity extends AppCompatActivity implements onImageClickListener{

    private RecyclerView mRecyclerView;
    private ArrayList<Bitmap> images;
    private ArrayList<String> imageNames;

    private RecyclerView.LayoutManager mLayoutManager;
    private RecyclerView.ViewHolder mViewHolder;
    private RecyclerView.Adapter mAdapter;

    private void readFiles()
    {
        if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {

            if (shouldShowRequestPermissionRationale(Manifest.permission.READ_EXTERNAL_STORAGE)) {
                // Explain to the user why we need to read the contacts
            }

            requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, 1);
        }

        String root = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).toString()+ "/ece558";

        Log.d("My app", root);
        File myDir = new File(root);
        myDir.mkdirs();
        Queue<File> files = new LinkedList<>();
        files.addAll(Arrays.asList(myDir.listFiles()));

        for(int i = 0; i < files.size(); i++)
            Log.i("FILES: ", ((LinkedList<File>) files).get(i).toString());

        images = new ArrayList<>();
        imageNames = new ArrayList<>();

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        Bitmap bitmap;
        String s;

        for(int i = 0; i < files.size(); i++)
        {
            s = ((LinkedList<File>) files).get(i).toString();

            bitmap = BitmapFactory.decodeFile(s, options);
            s = s.replace(root+"/", "");
            s = s.replace(".png", "");
            imageNames.add(s);

            images.add(bitmap);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gallery);

        readFiles();

        mRecyclerView = findViewById(R.id.recyclerView);
        mLayoutManager = new GridLayoutManager(this, 3);
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setLayoutManager(mLayoutManager);

        mAdapter = new RecyclerAdapter(this);
        mRecyclerView.setAdapter(mAdapter);
    }

    @Override
    public void onImageClick(int position) {

        Bitmap b = images.get(position);
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        b.compress(Bitmap.CompressFormat.PNG, 100, stream);
        byte[] bytes = stream.toByteArray();

        Intent intent = new Intent(GalleryActivity.this, PhotoActivity.class);
        intent.putExtra("IMAGE", bytes);
        startActivity(intent);

    }

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

            View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.photo_layout, viewGroup, false);
            ImageViewHolder imageViewHolder = new ImageViewHolder(view, mOnImageClickListener);

            return imageViewHolder;
        }

        @Override
        public void onBindViewHolder(@NonNull ImageViewHolder viewHolder, int i) {

            Bitmap bitmap = images.get(i);
            String title = imageNames.get(i);

            viewHolder.image.setImageBitmap(bitmap);
            viewHolder.name.setText(title);

        }

        @Override
        public int getItemCount() {
            return imageNames.size();
        }
    }

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
