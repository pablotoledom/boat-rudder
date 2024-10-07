function updateUrlParam(param, value) {
    var url = window.location.href;
    var params = window.location.search.substring(1); // Get only the parameters (without the ?)
    var newParam = param + '=' + value;
    var updated = false;
    var paramList = params.split('&'); // Split parameters into an array

    // Loop through the parameters to check if the parameter exists
    for (var i = 0; i < paramList.length; i++) {
        if (paramList[i].indexOf(param + '=') === 0) {
            // Replace the parameter with the new value
            paramList[i] = newParam;
            updated = true;
            break;
        }
    }

    // If the parameter was not found, add it
    if (!updated) {
        paramList.push(newParam);
    }

    // Rebuild the query string
    var newQueryString = paramList.join('&');
    var newUrl = window.location.pathname + '?' + newQueryString;

    window.location.href = newUrl; // Redirect with the new URL
}
