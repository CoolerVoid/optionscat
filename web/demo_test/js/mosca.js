

function build_file_menu(json){


	jQuery.each(json, function(i, val) {
   $("#collapseMenu").append("<li class=\"list-group-item\"><a role=\"button\" data-toggle=\"collapse\" data-parent=\"#accordion\" href=\"#collapse\" aria-expanded=\"true\" aria-controls=\"#collapse\">"
   		+i+"</a></li>");
     jQuery.each(val, function(j, las) {
     		
     		
     });
});

	
}

function build_code_review(code, language, path){

    var path = "/usr/share/sample/code.c";
	var language = "c";
	$('#path').append(path);
	$('#code').append("<code class=\""+language+"\">"+code+"<code>");
	//refresh hightlight.js markup
	 $('pre code').each(
	 	function(i, block) {
    			hljs.highlightBlock(block);
  		});

}