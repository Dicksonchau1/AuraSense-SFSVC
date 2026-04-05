interface SectionHeadingProps {
  tag?: string;
  title: string;
  subtitle?: string;
  align?: 'left' | 'center';
}

export function SectionHeading({ tag, title, subtitle, align = 'left' }: SectionHeadingProps) {
  return (
    <div className={`mb-12 ${align === 'center' ? 'text-center' : ''}`}>
      {tag && (
        <span className="inline-block mb-3 text-xs font-mono font-medium tracking-widest uppercase text-accent-primary">
          {tag}
        </span>
      )}
      <h2 className="text-3xl md:text-4xl font-bold text-text-primary leading-tight mb-4">
        {title}
      </h2>
      {subtitle && (
        <p className={`text-lg text-text-secondary leading-relaxed ${align === 'center' ? 'max-w-3xl mx-auto' : 'max-w-2xl'}`}>
          {subtitle}
        </p>
      )}
    </div>
  );
}
