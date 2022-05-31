# Introduction


This is an [Apache HTTPD](https://httpd.apache.org/) module to enable a [BrAPI](https://brapi.org/) endpoint 
for a [Grassroots](https://grassroots.tools/) [Field Trials](https://github.com/TGAC/grassroots-service-field-trial)
service.

Currently there is limited support for the BrAPI 1.x specification for Studies, Trials, Locations and Traits. 
The plan is to update this to version 2.x and extend the support to cover more of the datatypes.


# Configuration

The only configuration parameter required is the web address of the Grassroots server running 
the Field Trials service that you wish to enable BrAPI support for. 
In the example below, we have a grassroots server running on *http://localhost:8080/grassroots/public*
and we specify that all requests beginning with *"/grassroots/brapi"* will be served by this Grassroots
BrAPI module.


```
LoadModule grassroots_brapi_module modules/mod_grassroots_brapi.so

#
# Set the uri for the Grassroots infrastructure requests
#
<Location "/grassroots/brapi">

        # Let Grassroots handle these requests
        SetHandler grassroots-brapi-handler

        GrassrootsURL http://localhost:8080/grassroots/public
</Location>
```

So for example a call to *http://localhost:8080/grassroots/brapi/brapi/v1/studies* will get all of the 
Studies in the system. See the [Get Studies](https://brapi.docs.apiary.io/#reference/study/studies/get-studies) 
endpoint for more information.
