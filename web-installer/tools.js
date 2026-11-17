// The tools this firmware carries, in the order the start menu shows them by
// default. Adding a tool to the site is one entry here; `id` must match the
// tool's id in kLauncherTools (TigerTagSplashESP32.ino), since it is what the
// page sends to the scale over USB ("TOOLS=pod,parcel").
//
// `accessory` (optional) is a part to print for the tool: its text in each
// language and the link to the file, once there is one.
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
    accessory: {
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
  }
];

// Line icons, 24x24, stroke = currentColor.
window.TOOL_ICONS = {
  spool: '<circle cx="12" cy="12" r="8.5"/><circle cx="12" cy="12" r="3"/><path d="M12 3.5v5.5M12 15v5.5"/>',
  chip:  '<rect x="6.5" y="6.5" width="11" height="11" rx="2"/><path d="M9.5 3v3.5M14.5 3v3.5M9.5 17.5V21M14.5 17.5V21M3 9.5h3.5M3 14.5h3.5M17.5 9.5H21M17.5 14.5H21"/>',
  box:   '<path d="M3.5 7.5 12 3l8.5 4.5v9L12 21l-8.5-4.5z"/><path d="M3.5 7.5 12 12l8.5-4.5M12 12v9"/>',
  gauge: '<path d="M4.5 17a8.5 8.5 0 1 1 15 0"/><path d="M12 13.5 15.5 9"/><circle cx="12" cy="14" r="1.2"/>',
  press: '<path d="M5 4h14v4H5z"/><path d="M12 8v5"/><path d="M4 18h16v2.5H4z"/><path d="M8 13h8l1 5H7z"/>'
};
