
singleton TSShapeConstructor(DefaulttreeDAE)
{
   baseShape = "./defaulttree.DAE";
};

function DefaulttreeDAE::onLoad(%this)
{
   %this.setDetailLevelSize("200", "40");
   %this.setDetailLevelSize("500", "80");
   %this.setDetailLevelSize("25", "5");
}
