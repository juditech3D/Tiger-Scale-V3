// The tools this firmware carries, in the order the start menu shows them by
// default. Adding a tool to the site is one entry here; `id` must match the
// tool's id in kLauncherTools (TigerTagSplashESP32.ino), since it is what the
// page sends to the scale over USB ("TOOLS=pod,parcel").
//
// `needs` (optional) is something the tool needs besides the firmware (a part
// to print, an app): its text in each language, the link once there is one,
// and the link's own label (default: "Download the file").
//
// `core` marks the tool that is always installed, always first: the spool
// workflow the scale was built for.
window.TOOLS = [
  {
    id: 'tigerscale', core: true, color: '#2f7fff', icon: 'spool',
    name: { fr: 'Tiger Scale', en: 'Tiger Scale' },
    desc: {
      fr: 'La pesée des bobines : lit la puce TigerTag, pèse, retire la tare de la bobine vide et envoie le reste à ton inventaire.',
      en: 'Spool weighing: reads the TigerTag chip, weighs, subtracts the empty spool and sends the rest to your inventory.'
    }
  },
  {
    id: 'pod', color: '#f5a623', icon: 'chip',
    needs: {
      url: 'https://github.com/juditech3D/TigerTag-Studio-Manager/releases/latest',
      label: { fr: 'Télécharger ce Tiger Studio', en: 'Download this Tiger Studio' },
      fr: 'Nécessite le Tiger Studio de ce fork (Windows, Linux ou macOS) : le Studio officiel ne sait pas utiliser la balance comme TigerPOD.',
      en: 'Needs this fork’s Tiger Studio (Windows, Linux or macOS): the official Studio cannot use the scale as a TigerPOD.'
    },
    name: { fr: 'TigerPOD', en: 'TigerPOD' },
    desc: {
      fr: 'Les deux lecteurs de la balance deviennent un TigerPOD pour Tiger Studio : lecture et écriture des puces par le réseau.',
      en: 'The scale\'s two readers become a TigerPOD for Tiger Studio: read and write chips over the network.'
    }
  },
  {
    id: 'parcel', color: '#e8821e', icon: 'box',
    // A printed part the tool needs. `url` stays empty until the file is
    // published; the page then says "coming soon" instead of a link.
    needs: {
      url: '',
      fr: 'Nécessite l’accessoire pèse-colis, à imprimer séparément.',
      en: 'Needs the parcel-scale accessory, printed separately.'
    },
    name: { fr: 'Pèse-colis', en: 'Parcel scale' },
    desc: {
      fr: 'Le poids en direct, en grammes, avec un bouton TARE. Sans compte ni réseau.',
      en: 'Live weight in grams with a TARE button. No account, no network.'
    }
  },
  {
    id: 'caltest', color: '#3ba55d', icon: 'gauge',
    name: { fr: 'Calibration & test', en: 'Calibration & test' },
    desc: {
      fr: 'Les valeurs brutes du capteur, leur stabilité, un diagnostic du câblage et l\'assistant de calibration.',
      en: 'Raw load-cell readings, their stability, a wiring diagnosis and the calibration wizard.'
    }
  },
  {
    id: 'presse', color: '#e24b4a', icon: 'press',
    name: { fr: 'Mémo presse', en: 'Heat press memo' },
    desc: {
      fr: 'Températures et temps de la presse à chaud par produit, avec minuteur et alarme. La liste se modifie depuis un navigateur.',
      en: 'Heat press temperatures and times per product, with a timer and alarm. The list is edited from a browser.'
    }
  },
  {
    id: 'count', color: '#9b59b6', icon: 'count',
    needs: {
      url: '',
      fr: 'Nécessite un bac de comptage posé sur la balance, à imprimer séparément.',
      en: 'Needs a counting tray on the scale, printed separately.'
    },
    name: { fr: 'Comptage de pièces', en: 'Parts counter' },
    desc: {
      fr: 'Pose le bac et fais la tare, prends une référence (1 à 100 pièces), puis verse : la balance compte les pièces identiques par leur poids.',
      en: 'Put the tray on and tare, take a reference (1 to 100 parts), then pour: the scale counts identical parts by their weight.'
    }
  },
  {
    id: 'mix', color: '#1abc9c', icon: 'mix',
    needs: {
      url: '',
      fr: 'Le gobelet se pose sur l’accessoire pèse-colis, à imprimer séparément.',
      en: 'The cup sits on the parcel-scale accessory, printed separately.'
    },
    name: { fr: 'Dosage résine', en: 'Resin mixing' },
    desc: {
      fr: 'Résine ou époxy en deux parties, au gramme : règle le rapport (100 : 45…), verse A, puis la balance dit combien de B ajouter, avec une barre et un bip.',
      en: 'Two-part resin or epoxy, to the gram: set the ratio (100 : 45…), pour A, then the scale tells how much B to add, with a bar and a beep.'
    }
  }
];

// How each tool works, in three parts: what it is for, what it can do, how to
// use it. The page shows them in the "How each tool works" section, one block
// per tool at #help-<id>, linked from its card. They describe the PUBLISHED
// firmware: a feature still on a branch goes in the "coming soon" section of
// index.html (SOON), never here.
window.TOOL_HELP = {
  "pod": {
    "fr": [
      "Utiliser les deux lecteurs de la balance comme un lecteur TigerPOD branché à l'ordinateur, pour travailler sur tes puces depuis Tiger Studio, sans câble.",
      "• ouvrir la bobine d'une puce posée\n• écrire une puce vierge\n• passer une TigerTag en TigerTag+\n• restaurer ou effacer une puce\n• lire les puces Bambu Lab",
      "1. Laisse cet outil ouvert sur la balance.\n2. Ouvre Tiger Studio sur le même réseau, avec le compte de la balance.\n3. Pose une puce sur un lecteur : Tiger Studio la voit tout de suite.\nL'écriture marche seulement tant que l'outil est ouvert."
    ],
    "en": [
      "Use the scale's two readers like a TigerPOD plugged into the computer, to work on your chips from Tiger Studio, with no cable.",
      "• open the spool of a chip put down\n• write a blank chip\n• turn a TigerTag into a TigerTag+\n• restore or erase a chip\n• read Bambu Lab chips",
      "1. Leave this tool open on the scale.\n2. Open Tiger Studio on the same network, with the scale's account.\n3. Put a chip on a reader: Tiger Studio sees it at once.\nWriting only works while this tool is open."
    ]
  },
  "parcel": {
    "fr": [
      "Peser un colis ou n'importe quel objet, en grammes, sans puce, sans compte et sans réseau.",
      "• le poids en direct, en grand\n• la tare, avec ou sans carton sur le plateau\n• le même zéro que l'écran Tiger Scale\n• l'écran se rallume tout seul quand le poids bouge",
      "1. Plateau vide, ou avec le carton vide : appuie sur TARE.\n2. Pose le colis.\n3. Lis le poids."
    ],
    "en": [
      "Weigh a parcel or any object, in grams, with no chip, no account and no network.",
      "• the live weight, in large digits\n• tare, with or without a box on the platform\n• the same zero as the Tiger Scale screen\n• the screen wakes up by itself when the weight moves",
      "1. Empty platform, or with the empty box: press TARE.\n2. Put the parcel on.\n3. Read the weight."
    ]
  },
  "caltest": {
    "fr": [
      "Vérifier que le capteur de poids marche bien, trouver d'où vient un poids faux ou qui saute, et recalibrer la balance.",
      "• voir les mesures brutes du capteur\n• un diagnostic qui nomme la panne probable : fil débranché, fil mal serré, mauvais contact\n• Référence : voir si le capteur réagit quand on appuie\n• lancer la calibration",
      "1. Plateau vide : lis le diagnostic.\n2. Touche Référence, puis appuie doucement sur le plateau : les chiffres doivent bouger.\n3. Pour recalibrer : touche Calibrer et suis les étapes avec un poids connu."
    ],
    "en": [
      "Check that the load cell works, find out why a weight is wrong or jumps, and recalibrate the scale.",
      "• see the load cell's raw readings\n• a diagnosis naming the likely fault: unplugged wire, loose wire, bad contact\n• Reference: see whether the load cell reacts when pressed\n• start the calibration",
      "1. Empty platform: read the diagnosis.\n2. Touch Reference, then press gently on the platform: the numbers must move.\n3. To recalibrate: touch Calibrate and follow the steps with a known weight."
    ]
  },
  "presse": {
    "fr": [
      "Retrouver d'un coup d'œil les réglages de ta presse à chaud pour chaque produit : température, temps, pression et pelage.",
      "• tes produits rangés par technique (sublimation, DTF, flex, flocage…)\n• un minuteur de pressage lancé d'un toucher\n• une alarme à la fin, qui rappelle le pelage\n• ta propre liste, modifiable depuis l'ordinateur",
      "1. Choisis une technique, puis un produit.\n2. Touche Démarrer en fermant la presse : le temps défile et sonne à la fin.\n3. Pour changer la liste : ouvre la page presse de la balance dans ton navigateur.\nLes valeurs d'usine sont indicatives : vérifie les fiches de tes fournisseurs."
    ],
    "en": [
      "Find your heat press settings for each product at a glance: temperature, time, pressure and peel.",
      "• your products sorted by technique (sublimation, DTF, flex, flock...)\n• a pressing timer started with one touch\n• an alarm at the end, reminding you of the peel\n• your own list, edited from the computer",
      "1. Pick a technique, then a product.\n2. Touch Start as you close the press: the time counts down and rings at the end.\n3. To change the list: open the scale's press page in your browser.\nFactory values are a guide: check your suppliers' sheets."
    ]
  },
  "count": {
    "fr": [
      "Compter des pièces identiques (vis, écrous, inserts, perles…) en les pesant, au lieu de les compter à la main.",
      "• une référence de 1 à 100 pièces\n• une vérification qui rend le compte plus juste\n• la balance garde le poids d'une pièce : « Mêmes pièces » la fois suivante\n• ajoute ou retire des pièces, le compte suit",
      "1. Pose le bac vide et appuie sur TARE.\n2. Pose le nombre de pièces choisi, puis Valider.\n3. Ajoute autant de pièces pour vérifier, puis « C'est bon, compter ».\nAstuce : si la balance dit que la référence est légère, prends plus de pièces."
    ],
    "en": [
      "Count identical parts (screws, nuts, inserts, beads...) by weighing them instead of counting by hand.",
      "• a reference of 1 to 100 parts\n• a check that makes the count more accurate\n• the scale keeps the weight of one part: \"Same parts\" next time\n• add or remove parts, the count follows",
      "1. Put the empty tray on and press TARE.\n2. Put the chosen number of parts on, then Confirm.\n3. Add as many parts again to check, then \"Correct, count\".\nTip: if the scale says the reference is light, take more parts."
    ]
  },
  "mix": {
    "fr": [
      "Doser une résine en deux parties (époxy, polyuréthane, silicone…) au bon rapport, au gramme près.",
      "• le rapport A : B réglé une fois, gardé pour la fois suivante\n• le poids de B calculé d'après A\n• une barre, un bip quand c'est bon, du rouge si c'est trop",
      "1. Pose le gobelet sur l'adaptateur, puis appuie sur TARE.\n2. Règle le rapport A : B, verse A, puis « Passer à B ».\n3. Verse B jusqu'au bip."
    ],
    "en": [
      "Measure a two-part resin (epoxy, polyurethane, silicone...) at the right ratio, to the gram.",
      "• the A : B ratio set once, kept for next time\n• the weight of B worked out from A\n• a bar, a beep when it is right, red when it is too much",
      "1. Put the cup on the adapter, then press TARE.\n2. Set the A : B ratio, pour A, then \"Go to B\".\n3. Pour B until the beep."
    ]
  }
};

// Line icons, 24x24, stroke = currentColor.
window.TOOL_ICONS = {
  spool: '<circle cx="12" cy="12" r="8.5"/><circle cx="12" cy="12" r="3"/><path d="M12 3.5v5.5M12 15v5.5"/>',
  chip:  '<rect x="6.5" y="6.5" width="11" height="11" rx="2"/><path d="M9.5 3v3.5M14.5 3v3.5M9.5 17.5V21M14.5 17.5V21M3 9.5h3.5M3 14.5h3.5M17.5 9.5H21M17.5 14.5H21"/>',
  box:   '<path d="M3.5 7.5 12 3l8.5 4.5v9L12 21l-8.5-4.5z"/><path d="M3.5 7.5 12 12l8.5-4.5M12 12v9"/>',
  gauge: '<path d="M4.5 17a8.5 8.5 0 1 1 15 0"/><path d="M12 13.5 15.5 9"/><circle cx="12" cy="14" r="1.2"/>',
  press: '<path d="M5 4h14v4H5z"/><path d="M12 8v5"/><path d="M4 18h16v2.5H4z"/><path d="M8 13h8l1 5H7z"/>',
  count: '<path d="M4 9h16l-1.5 10.5h-13z"/><circle cx="9" cy="6" r="1.6"/><circle cx="14.5" cy="5" r="1.6"/><path d="M10 13h4M12 11v4"/>',
  mix:   '<path d="M6 4h5l-1 15H7z"/><path d="M13 4h5l-1 15h-3z"/><path d="M6.5 10h4M13.5 12h4"/>'
};
