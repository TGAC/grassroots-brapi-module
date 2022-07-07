# Introduction


This is an [Apache HTTPD](https://httpd.apache.org/) module to enable a [BrAPI](https://brapi.org/) endpoint 
for a [Grassroots](https://grassroots.tools/) [Field Trials](https://github.com/TGAC/grassroots-service-field-trial)
service.

Currently there is limited support for the BrAPI 2.x specification for Studies, Trials, Locations and Programs. 


# Building the module

To build the module you need to create a ```build/linux/user.prefs``` 
file. To do this you can copy and edit the example file

```
cp build/linux/example-user.prefs build/linux/user.prefs
```

And then edit the values in there to match your system. 

# Configuration

The following configuration parameters are available:

* **GrassrootsServerURL**:
This is the web address of the Grassroots backend server running 
the Field Trials service that you wish to enable BrAPI support for. This value is required.

* **GrassrootsFrontEndURL**:
This is the root web address of the Grassroots frontend [Django-based server](https://github.com/TGAC/grassroots_services_django_web)
which is displaying the field trial data

* **GrassrootsFrontEndStudiesPath**:
The relative path from the value specified for *GrassrootsFrontEndURL* for accessing studies on the front end. 
By default this is set to *studies/*.

* **GrassrootsFrontEndTrialsPath**:
The relative path from the value specified for *GrassrootsFrontEndURL* for accessing trials on the front end.
By default this is set to *trials/*.

In the example below, we have a grassroots server running on *http://localhost:2000/grassroots/public*
and we specify that all requests beginning with *"/grassroots/brapi"* will be served by this Grassroots
BrAPI module. We have the frontend server tunning at http://localhost:8000/grassroots/fieldtrials.


```
LoadModule grassroots_brapi_module modules/mod_grassroots_brapi.so

#
# Set the uri for the Grassroots infrastructure requests
#
<Location "/grassroots/brapi">

        # Let Grassroots handle these requests
        SetHandler grassroots-brapi-handler

        GrassrootsServerURL http://localhost:2000/grassroots/public

        GrassrootsFrontEndURL http://localhost:8000/grassroots/fieldtrials
        
</Location>
```

So for example a call to *http://localhost:2000/grassroots/brapi/brapi/v2/studies* will get all of the 
Studies in the system. See the [Get Studies](https://brapi.docs.apiary.io/#reference/study/studies/get-studies) 
endpoint for more information.
