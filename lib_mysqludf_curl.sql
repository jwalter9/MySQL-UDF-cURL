
DROP FUNCTION IF EXISTS curl;
DROP FUNCTION IF EXISTS curl_buffered;
DROP FUNCTION IF EXISTS url_encode;

CREATE FUNCTION curl RETURNS string SONAME 'lib_mysqludf_curl.so';
CREATE FUNCTION curl_buffered RETURNS string SONAME 'lib_mysqludf_curl.so';
CREATE FUNCTION url_encode RETURNS string SONAME 'lib_mysqludf_curl.so';

