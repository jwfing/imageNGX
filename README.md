ImageNGX is combination of imagemagick and opencv running on EC2's.

## Usage

Assume we host the image service to img.yourdomain.com

### Accessing image:

#### Original

>##### URL: `http://img.yourdomain.com/v2/{bucket}/{file-name}?hs=`

>##### Parameter:
>* `bucket`: self-explanatory
>* `file-name`: self-explanatory
>* `hs`: defines the form that the uri of stored files on amazon S3.
 * `hs` = 0: the s3 uri are `${protocol}`://s3.amazonaws.com/`${bucket}`/`[${key}]`
 * `hs` = 1: the s3 uri are `${protocol}`://`${bucket}`.s3.amazonaws.com/`[${key}]`

> **NOTE:** `hs` default hs is 0.

>##### Sample:

#### Cropped

>##### URL: `http//img.yourdomain.com/v2/{bucket}/{file-name}?w=&h=`

>##### Paramters:
>* `bucket`: self-explanatory
>* `file-name`: self-explanatory
>* `w`: the width user want to be cropped
>* `h`: the height user want to be cropped

>**Note:** If either width or height is not specified, the image will scale
>proportionally to the dimension provided.

>##### Sample:

#### Format

>##### URL: `http://img.yourdomain.com/v2/{bucket}/{file-name}?fm=`

>##### Paramters:
>* `bucket`: self-explanatory
>* `file-name`: self-explanatory
>* fm: the format user want to be converted like jpg

>**Note:** it's better not to convert image from jpg to png, which may
>cause memory not enough.

>##### Sample:

#### Enhance:

>##### URL: `http://img.yourdomain.com/v2/{bucket}/{file-name}?bri=&sat=&hue=&exp=`

>##### Paramters:
>* `bucket`: self-explanatory
>* `file-name`: self-explanatory
>* `bri`: brightness
>* `sat`: saturation
>* `hue`: hue
>* `exp`: exposure

>**Note:** the values except exposure are all valid between 0 to 200. 100 means no change.
>`exp` is valid between -100 to 100. 0 means no change.

>##### Sample:

## Deploy

### Manully install image service on your own machine
>  * Clone the code by `git clone git@github.com:jwfing/imageNGX.git`
>  * `cd thirdparty & sudo ./prepare.sh` to install the dependence
>  * Have a coffe until you successful installed the
>    dependence
>  * Contact <jfeng@leancloud.rocks> if there is any problem above
>  * (Optional) Please check where the mongo-dev lib installed, and
>    the permission, you'd better make it readable by the current user
>  * Go to the project root dir, and `make && make test`
>  * Start service by `cd webservice/out && ./scripts/start_service.sh` 
 

 
 
    
