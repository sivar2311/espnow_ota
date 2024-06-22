#include "webPage.h"

const char* webPage = R"(
<html>
<body>
  <form method="post" enctype="multipart/form-data" action="/send_update">
    <input name="filename" type="file" size="50" accept=".bin"><br>
    <button>Upload</button>
  </form>
  </body>
</html>
)";
