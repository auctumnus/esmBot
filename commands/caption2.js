const magick = require("../utils/image.js");
const words = ["me irl", "dank", "follow my second account @esmBot_", "2016", "meme", "wholesome", "reddit", "instagram", "twitter", "facebook", "fortnite", "minecraft", "relatable", "gold", "funny", "template", "hilarious", "memes", "deep fried", "2020", "leafy", "pewdiepie"];

exports.run = async (message, args) => {
  const image = await require("../utils/imagedetect.js")(message);
  if (image === undefined) return `${message.author.mention}, you need to provide an image/GIF to add a caption!`;
  const newArgs = args.filter(item => !item.includes(image.url) );
  const processMessage = await message.channel.createMessage("<a:processing:479351417102925854> Processing... This might take a while");
  const { buffer, type } = await magick.run({
    cmd: "captionTwo",
    path: image.path,
    caption: newArgs.length !== 0 ? newArgs.join(" ").replace(/&/g, "\\&amp;").replace(/>/g, "\\&gt;").replace(/</g, "\\&lt;").replace(/"/g, "\\&quot;").replace(/'/g, "\\&apos;") : words.sort(() => 0.5 - Math.random()).slice(0, Math.floor(Math.random() * words.length + 1)).join(" ")
  });
  if (processMessage.channel.messages.get(processMessage.id)) await processMessage.delete();
  return {
    file: buffer,
    name: `caption2.${type}`
  };
};

exports.aliases = ["tags2", "meirl", "memecaption", "medotmecaption"];
exports.category = 5;
exports.help = "Adds a me.me caption/tag list to an image/GIF";
